#include <cstdio>
#include <stdexcept>
#include <string>  // getline, to_string
//#include <vector>
#include <unordered_map>
#include <limits>
#include <cstring>  // strcmp, strtok
#include <cctype>  // tolower
#include <fstream>  // ifstream
#include <cassert>
#include <algorithm>  // lower_bound
#include "Optimiser.h"
#include "cmdline.h"

using std::string;
//using std::vector;
using std::unordered_map;
using std::ifstream;
//using std::numeric_limits
using std::invalid_argument;
using std::domain_error;
using std::lower_bound;  // Binary search if applicable
using std::distance;
using std::to_string;


//! Base of the ids (decimal by default)
const uint8_t  ID_BASE = 10;
//! Mapping of the internal to the ordered external ids
using Nodes = vector<Id>;  // Note: to store them as igraph node vertex attribute, the type should be compatible with igraph_real_t
////! The maximal number of decimal digits in Id type
//constexpr uint8_t  ID_DIG10 = log10(numeric_limits<Id>::max() - 1) + 1;
////! Map from the external to the internal node ids, internal ids correspond the vector index
//using ExternIds = vector<Id>  extids;


//! \brief Igraph vector view for custom types
template<typename BASE>
const igraph_vector_t* igraph_vector_view(const igraph_vector_t *v, const BASE *data, Id length) noexcept
{
	static_assert(sizeof(igraph_real_t) == sizeof(BASE), "BASE type should be compatible with the igraph_real_t");
	assert(data && "Allocated data array is expected");
	auto vv = const_cast<igraph_vector_t*>(v);

	vv->stor_begin = reinterpret_cast<igraph_real_t*>(const_cast<BASE*>(data));
	vv->stor_end = reinterpret_cast<igraph_real_t*>(const_cast<BASE*>(data)) + length;
	vv->end = vv->stor_end;
	return v;
}


//! \brief Fetch existing of create a new node by the external id
//!
//! \param nodes Nodes&  - mapping of the internal node ids to the ordered external ids
//! \param eid id  - external id to fetch the respective node or create if not existed
//! \return Id  - resulting internal node id
Id getNode(Nodes& nodes, Id eid) noexcept  // Note: there are no any catchable exceptions
{
	auto ie = nodes.end();
	if(eid < nodes.size()) {
		if(nodes[eid] == eid)
			return eid;
		ie = nodes.begin() + eid;
	}
	auto ib = lower_bound(nodes.begin(), ie, eid);
	if(ib == nodes.end()) {
		nodes.push_back(eid);
		return nodes.size() - 1;  // Index of the last item
	}
	if(*ib != eid)
		ib = nodes.insert(ib, eid);
	assert(*ib > eid && "The external ids of the nodes should be ASC ordered");
	return distance(nodes.begin(), ib);
}


//! \brief Convert text the the lowercase
//!
//! \param text char*  - text to be converted
//! \return void
void tolower(char* text)
{
	if(!text)
		return;
	do
		*text = tolower(*text);
	while(*++text);
}


//! \brief Load graph from the NSL (A/E) or NCOL file
//! \pre Node ids should have uint_32 type, may form non-contiguous ranges and start from any number
//!
//! \param inpfile string  - input file name
//! \param directed=-1 int8_t  - whether the input network is directed ({-1, 0, 1}),
//! 	-1 means identify automatically by the file header
//! \return igraph_t  - resulting graph with the constructed vertices, edges and possible attributes:
//! 	`name` vertices attribute contains external node ids, it is present only if the internal ids differ
//! 	`weight` links attribute contains link weights, it is present only if the graph is weighted
//! 	\note Whether the graph is directed can be found by the `igraph_bool_t igraph_is_directed(const igraph_t *graph)`
////! \param[out] extids ExternIds&  - vector of the external ids, which is empty if the external
////! 	ids are equal to the internal ids
igraph_t loadGraphNSL(string inpfile, int8_t directed=-1)
{
	if(directed == -1) {
		// Use extension to identify the file format
		Id iext = inpfile.rfind('.');
		if(iext != string::npos) {
			if(!strcmp(&inpfile.c_str()[iext+1], "nse"))
				directed = false;
			else if(!strcmp(&inpfile.c_str()[iext+1], "nsa"))
				directed = true;
		}
	}

	ifstream  finp;
	finp.exceptions(ifstream::badbit);  //  | ifstream::failbit  - raises exception on EOF
	finp.open(inpfile);
	if(!finp.is_open()) {
		perror(("Error opening the file: " + inpfile).c_str());
		throw std::ios_base::failure(strerror(errno));
	}

	// Parse the NSE/A file header
	// [Nodes: <nodes_num>[,]	<Links>: <links_num>[,] [Weighted: {0, 1}]]
	// Note: the comma is either always present as a delimiter or always absent
	Id  n = 0;  // The specified number of nodes
	Id  m = 0;  // The specified number of links
	int8_t  weighted = -1;  // The network is weighted, -1 means not specified
	string  line;  // Parsing line of the file

	while(getline(finp, line)) {
		// Skip empty lines
		if(line.empty())
			continue;
		// Consider only subsequent comments
		if(line[0] != '#')
			break;

		//// 1. Replace the staring comment mark '#' with space to allow "#nodes:"
		//line[0] = ' ';
		// 2. Replace ':' with space to allow "Nodes:<ndsnum>"
		for(Id pos = 0; pos != string::npos; pos = line.find(':', pos + 1))
			line[pos] = ' ';

		// Parse nodes num
		char *tok = strtok(const_cast<char*>(line.data()) + 1, " \t");  // Note: +1 to omit the starting '#'
		if(!tok)
			continue;
		tolower(tok);
		if(strcmp(tok, "nodes"))
			continue;
		// Read nodes num
		tok = strtok(nullptr, " \t");
		if(tok) {
			// Note: optional trailing ',' is allowed here
			n = strtoul(tok, nullptr, ID_BASE);
			// Read the number of links
			tok = strtok(nullptr, " \t");
			if(tok) {
				tolower(tok);
				if(directed != 0 && !strcmp(tok, "arcs"))
					directed = 1;
				else if(directed <= 0 && !strcmp(tok, "edges"))
					directed = 0;
				else throw domain_error(string("The file format (").append(tok)
					.append(") is either inconsistent with the expected one (directed: ")
					+= to_string(directed).append(")\n"));
				tok = strtok(nullptr, " \t");
				if(tok) {
					// Note: optional trailing ',' is allowed here
					m = strtoul(tok, nullptr, ID_BASE);
					// Read Weighted flag
					tok = strtok(nullptr, " \t");
					if(tok && (tolower(tok), !strcmp(tok, "weighted")) && (tok = strtok(nullptr, " \t"))) {
						weighted = strtoul(tok, nullptr, ID_BASE);
						if(weighted < 0)
							throw invalid_argument(string("Invalid value of weighted: ").append(tok) += '\n');
					}
				}
			}
		}
	}
	assert(directed >= 0 && directed <= 1 && "Links identification is failed");

	Nodes  nodes;  // Mapping of the internal to the external node ids
	vector<Id>  links;  // Links specified by the internal ids by pairs of the elements: (from, to)
	vector<Weight>  weights;  // Link weights, should be synced with the links container
	if(n) {
		nodes.reserve(n);
		if(m) {
			links.reserve(m*2);
			weights.reserve(m);
		}
	}

	// Parse the body
	// Note: the processing is started from the read line
	Id  iline = 0;  // Payload line index, links counter
	do {
		// Skip empty lines and comments
		if(line.empty() || line[0] == '#')
			continue;

		char *tok = strtok(const_cast<char*>(line.data()), " \t");
		if(!tok)
			continue;
		Id nid = strtoul(tok, nullptr, ID_BASE);  // External source id
		links.push_back(getNode(nodes, nid));
		tok = strtok(nullptr, " \t");
		if(!tok)
			throw invalid_argument(string("Destination link id is not specified in this line: ") += line);
		nid = strtoul(tok, nullptr, ID_BASE);  // External destination id
		links.push_back(getNode(nodes, nid));
		// Parse weight if required
		if(weighted) {
			tok = strtok(nullptr, " \t");
			if(tok) {
				weighted = 1;  // The network is definitely weighted => -1 -> 1
				const Weight  lw = strtof(tok, nullptr);  // Note: typically, float is a sufficient accuracy of the input weight
				if(!lw && tok[0] != '0' && (tok[0] != '-' || tok[1] != '0'))  // Note: in the worst case tok[1] == 0 (EOL)
					throw invalid_argument(string("Invalid link weight: ") += tok);
				weights.push_back(lw);
			} else if(weighted >= 1)
					throw invalid_argument(string("Link weight is not specified in this line: ") += line);
		}
		++iline;
	} while(getline(finp, line));
	assert((weights.empty() || weights.size() * 2 == links.size())
		&& "Link weights should be synchronized with the links");

	// Initialize the graph
	igraph_t  graph;
	auto err = igraph_empty(&graph, nodes.size(), directed);
	if(err)
		throw LeidenException("Graph nodes construction failed: " + to_string(err));

	igraph_vector_t  igvec;  // Igraph vector view
	// Save external (original) node ids if they are not the same as the internal ids
	if(!nodes.empty() && nodes.back() == nodes.size() - 1) {
		err = SETVANV(&graph, "name", igraph_vector_view(&igvec, nodes.data(), nodes.size()));
		if(err)
			throw LeidenException("Graph weights node names (ext ids) assignment is failed: " + to_string(err));
	}
	// Fill the graph links
	err = igraph_add_edges(&graph, igraph_vector_view(&igvec, links.data(), links.size()), nullptr);
	if(err)
		throw LeidenException("Graph links construction failed: " + to_string(err));
	// Fill the graph weights
	if(!weights.empty()) {
		err = SETEANV(&graph, "weight", igraph_vector_view(&igvec, weights.data(), weights.size()));
		if(err)
			throw LeidenException("Graph weights assignment is failed: " + to_string(err));
	}
	return graph;
}


int main(int argc, char* argv[])
{
	// Parse input arguments
	gengetopt_args_info args_info;
	auto err = cmdline_parser(argc, argv, &args_info);
	if(err)
		return err;

	if(args_info.inputs_num != 1) {
		fputs("Error: Input network is expected as a path (file / directory)\n\n", stderr);
		cmdline_parser_print_help();
		return 1;
	}
	printf("Starting tbe algorithm (gamma: %g)\n\tinput (%s): %s\n\toutput (%s): %s\n"
		, args_info.gamma_arg
		, args_info.inp_fmt_orig, args_info.inputs[0]
		, args_info.res_fmt_orig, args_info.output_arg);

	// Load the input graph
	Graph  gr(loadGraphNSL(args_info.inputs[0], args_info.inp_fmt_given
		? args_info.inp_fmt_arg == inp_fmt_arg_NSA: -1));

	// Perform the clustering
	Optimiser  opt;
	if(args_info.seed_given)
		opt.set_rng_seed(args_info.seed_arg);

	//MutableVertexPartition  part(&gr);
//	uint16_t  nopts = args_info.optim_iters_arg;
//	if(args_info.gamma_given) {
//		MutableVertexPartition  part = opt.template find_partition(&gr, args_info.gamma_arg);
//		//auto dmod = opt.optimise_partition(&part);
//		//optimize(part, opt, nopts);
//		Weight  dmod = 0;
//		while(nopts--)
//			dmod += opt.optimise_partition(&part);
//		//part.membership();  // Vector indicating community id for each node
//	} else {
//		MutableVertexPartition  part = opt.find_partition(&gr);
//		//auto mod =
//		opt.optimise_partition(&part);
//	}

//	if(args_info.output_given) {
//		printf("Saving the resulting clustering into: %s\n", args_info.output_arg);
//		graph->output(args_info.output_arg, args_info.legacy_flag);
//	}

	return 0;
}
