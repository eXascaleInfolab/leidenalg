#ifndef GRAPHHELPER_INCLUDED
#define GRAPHHELPER_INCLUDED

#include <igraph.h>
#include <vector>
#include <set>
#include <exception>
#include <queue>

//#ifdef DEBUG
#include <iostream>
  using std::cerr;
  using std::endl;
//#endif

using std::vector;
using std::string;
using std::pair;
using std::set;
using std::queue;
using std::make_pair;


//! Id type
using Id = uint64_t;  // Note: 64 bit type is required for the efficient applicability with igraph_vector_t containers
static_assert(sizeof(Id) == sizeof(igraph_real_t), "Id should be compatible with igraph_real_t");

//! Link weight type
using Weight = double;  // Note: should be consistent with the internal weight of graph links (with both Leiden Graph and igraph_real_t that is required for efficient applicability with igraph_vector_t)
static_assert(sizeof(Weight) == sizeof(igraph_real_t), "Weight should be compatible with igraph_real_t");

class MutableVertexPartition;

vector<Id> range(Id n);
queue<Id> queue_range(Id n);

bool orderCSize(const Id* A, const Id* B);

Weight KL(Weight q, Weight p);
Weight KLL(Weight q, Weight p);

template <class T> T sum(vector<T> vec)
{
  T sum_of_elems = T();
  for(typename vector<T>::iterator it=vec.begin();
      it!=vec.end();
      it++)
      sum_of_elems += *it;
  return sum_of_elems;
};

struct LeidenException: std::runtime_error
{
    LeidenException(const string& str="") noexcept: std::runtime_error(str)  {}
    LeidenException(const LeidenException&)=default;
    LeidenException(LeidenException&&)=default;
    virtual ~LeidenException()=default;
};

inline Id get_random_int(Id from, Id to, igraph_rng_t* rng) noexcept
{
  return igraph_rng_get_integer(rng, from, to);
};

void shuffle(vector<Id>& v, igraph_rng_t* rng);

class Graph
{
  public:
    Graph(igraph_t* graph,
      vector<Weight> const& edge_weights,
      vector<Id> const& node_sizes,
      vector<Weight> const& node_self_weights, int correct_self_loops);
    Graph(igraph_t* graph,
      vector<Weight> const& edge_weights,
      vector<Id> const& node_sizes,
      vector<Weight> const& node_self_weights);
    Graph(igraph_t* graph,
      vector<Weight> const& edge_weights,
      vector<Id> const& node_sizes, int correct_self_loops);
    Graph(igraph_t* graph,
      vector<Weight> const& edge_weights,
      vector<Id> const& node_sizes);
    Graph(igraph_t* graph, vector<Weight> const& edge_weights, int correct_self_loops);
    Graph(igraph_t* graph, vector<Weight> const& edge_weights);
    Graph(igraph_t* graph, vector<Id> const& node_sizes, int correct_self_loops);
    Graph(igraph_t* graph, vector<Id> const& node_sizes);
    Graph(igraph_t* graph, int correct_self_loops);
    Graph(igraph_t* graph);
    Graph();

    // C++11+ constructors
    //! \brief Graph construction from the fully initialized attributed igraph
    //!
    //! \param gr igraph_t&&  - fully initialized attributed graph to be wrapped
    Graph(igraph_t&& gr) noexcept;
    Graph(const Graph& other)=delete;
    Graph(Graph&& other) noexcept;

    ~Graph();

    Graph& operator=(const Graph&)=delete;
    Graph& operator=(Graph&& other) noexcept;

    int has_self_loops() const;
    //! The maximal number of edges
    Id possible_edges() const noexcept;
    //! The maximal number of edges for the specified number of vertices
    //! considering whether the graph is directed
    Id possible_edges(Id n) const noexcept;

    Graph* collapse_graph(MutableVertexPartition* partition) const;

    vector<Id> const& get_neighbour_edges(Id v, igraph_neimode_t mode) const noexcept;
    vector<Id> const& get_neighbours(Id v, igraph_neimode_t mode) const;
    Id get_random_neighbour(Id v, igraph_neimode_t mode, igraph_rng_t* rng) const;

    pair<Id, Id> get_endpoints(Id e) const noexcept;

    inline Id get_random_node(igraph_rng_t* rng) const noexcept
    {
      return get_random_int(0, this->vcount() - 1, rng);
    };

    inline const igraph_t* get_igraph() const noexcept  { return this->_graph; };
    //inline igraph_t* get_igraph() noexcept  { return this->_graph; };

    inline Id vcount() const noexcept  { return igraph_vcount(this->_graph); };
    inline Id ecount() const noexcept { return igraph_ecount(this->_graph); };
    inline Weight total_weight() const noexcept { return this->_total_weight; };
    inline Id total_size() const noexcept { return this->_total_size; };
    inline int is_directed() const noexcept { return igraph_is_directed(this->_graph); };
    inline Weight density() const noexcept { return this->_density; };
    inline int correct_self_loops() const noexcept { return this->_correct_self_loops; };
    inline int is_weighted() const noexcept { return this->_is_weighted; };

    // Get weight of edge based on attribute (or 1.0 if there is none).
    inline Weight edge_weight(Id e) const
    #ifndef DEBUG
      noexcept
    #endif
    {
      #ifdef DEBUG
      //return _edge_weights.at(e);
      if (e > this->_edge_weights.size())
        throw LeidenException("Edges outside of range of edge weights.");
      #endif
      return this->_edge_weights[e];
      //return EAN(this, "weight", e);  // Note: igraph attributes processing is relatively slow
    };

    inline vector<Id> edge(Id e) const noexcept
    {
      igraph_integer_t v1, v2;
      igraph_edge(this->_graph, e, &v1, &v2);
      vector<Id> edge(2);
      edge[0] = v1; edge[1] = v2;
      return edge;
    }

    // Get size of node based on attribute (or 1.0 if there is none).
    inline Id node_size(Id v) const noexcept
    { return this->_node_sizes[v]; };

    // Get self weight of node based on attribute (or set to 0.0 if there is none)
    inline Weight node_self_weight(Id v) const noexcept
    { return this->_node_self_weights[v]; };

    inline Id degree(Id v, igraph_neimode_t mode) const
    {
      if (mode == IGRAPH_IN)
        return this->_degree_in[v];
      else if (mode == IGRAPH_OUT)
        return this->_degree_out[v];
      else if (mode == IGRAPH_ALL)
        return this->_degree_all[v];
      else
        throw LeidenException("Incorrect mode specified.");
    };

    inline Weight strength(Id v, igraph_neimode_t mode) const
    {
      if (mode == IGRAPH_IN)
        return this->_strength_in[v];
      else if (mode == IGRAPH_OUT)
        return this->_strength_out[v];
      else
        throw LeidenException("Incorrect mode specified.");
    };

  private:
    igraph_t* _graph;

  //protected:
    int _remove_graph;

    // Utility variables to easily access the strength of each node
    vector<Weight> _strength_in;
    vector<Weight> _strength_out;

    vector<Id> _degree_in;
    vector<Id> _degree_out;
    vector<Id> _degree_all;

    // Used for the weight of the edges because the igraph (edge and vertex) attributes access is inefficient
    vector<Weight> _edge_weights;
    vector<Id> _node_sizes; // Used for the size of the nodes.
    vector<Weight> _node_self_weights; // Used for the self weight of the nodes.

    void cache_neighbours(Id v, igraph_neimode_t mode) const noexcept;
    mutable vector<Id> _cached_neighs_from; mutable Id _current_node_cache_neigh_from;
    mutable vector<Id> _cached_neighs_to;   mutable Id _current_node_cache_neigh_to;
    mutable vector<Id> _cached_neighs_all;  mutable Id _current_node_cache_neigh_all;

    void cache_neighbour_edges(Id v, igraph_neimode_t mode) const noexcept;
    mutable vector<Id> _cached_neigh_edges_from; mutable Id _current_node_cache_neigh_edges_from;
    mutable vector<Id> _cached_neigh_edges_to;   mutable Id _current_node_cache_neigh_edges_to;
    mutable vector<Id> _cached_neigh_edges_all;  mutable Id _current_node_cache_neigh_edges_all;

    Weight _total_weight;
    Id _total_size;
    int _is_weighted;

    // Note: _correct_self_loops and _density are not used in the internal evaluations at all
    //! Consider node weights (self-links) on normalization for the density calculation
    int _correct_self_loops;
    Weight _density;

    void init_admin();
    void set_defaults();
    void set_default_edge_weight();
    void set_default_node_size();
    void set_self_weights();

};

// We need this ugly way to include the MutableVertexPartition
// to overcome a circular linkage problem.
#include "MutableVertexPartition.h"

#endif // GRAPHHELPER_INCLUDED

