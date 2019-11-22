#include <string>  // to_string
#include <cassert>
#include <type_traits>
#include "GraphHelper.h"
#include "MutableVertexPartition.h"

#ifdef DEBUG
  using std::cerr;
  using std::endl;
#endif

using std::to_string;
using std::move;
using std::remove_pointer_t;


vector<Id> range(Id n)
{
  vector<Id> range_vec(n);
  for(Id i = 0; i<n; i++)
    range_vec[i] = i;
  return range_vec;
}

queue<Id> queue_range(Id n)
{
  queue<Id> range_vec;
  for(Id i = 0; i<n; i++)
    range_vec.push(i);
  return range_vec;
}

bool orderCSize(const Id* A, const Id* B)
{

  if (A[1] == B[1])
  {
    if (A[2] == B[2])
      return A[0] < B[0];
    else
      return A[2] > B[2];
  }
  else
    return A[1] > B[1];
}

void shuffle(vector<Id>& v, igraph_rng_t* rng)
{
  Id n = v.size();
  for (Id idx = n - 1; idx > 0; idx--)
  {
    Id rand_idx = get_random_int(0, idx, rng);
    Id tmp = v[idx];
    v[idx] = v[rand_idx];
    v[rand_idx] = tmp;
  }
}

/****************************************************************************
  The binary Kullback-Leibler divergence.
****************************************************************************/
Weight KL(Weight q, Weight p)
{
  Weight KL = 0.0;
  if (q > 0.0 && p > 0.0)
    KL += q*log(q/p);
  if (q < 1.0 && p < 1.0)
    KL += (1.0-q)*log((1.0-q)/(1.0-p));
  return KL;
}

Weight KLL(Weight q, Weight p)
{
  Weight KL = 0.0;
  if (q > 0.0 && p > 0.0)
    KL += q*log(q/p);
  if (q < 1.0 && p < 1.0)
    KL += (1.0-q)*log((1.0-q)/(1.0-p));
  if (q < p)
    KL *= -1;
  return KL;
}


Graph::Graph(igraph_t* graph, vector<Weight> const& edge_weights
  , vector<Id> const& node_sizes, vector<Weight> const& node_self_weights
  , int correct_self_loops): _graph(graph), _remove_graph(false), _owner(nullptr)
{
  if (edge_weights.size() != this->ecount())
    throw LeidenException("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw LeidenException("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  if (node_self_weights.size() != this->vcount())
    throw LeidenException("Node self weights vector inconsistent length with the vertex count of the graph.");
  this->_node_self_weights = node_self_weights;

  this->_correct_self_loops = correct_self_loops;
  this->init_admin();
}

Graph::Graph(igraph_t* graph, vector<Weight> const& edge_weights
  , vector<Id> const& node_sizes, vector<Weight> const& node_self_weights)
  : _graph(graph), _remove_graph(false), _owner(nullptr)
{
  if (edge_weights.size() != this->ecount())
    throw LeidenException("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw LeidenException("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->_correct_self_loops = this->has_self_loops();

  this->_node_self_weights = node_self_weights;
  this->init_admin();
}

Graph::Graph(igraph_t* graph, vector<Weight> const& edge_weights
  , vector<Id> const& node_sizes, int correct_self_loops)
  : _graph(graph), _remove_graph(false), _owner(nullptr)
{
  if (edge_weights.size() != this->ecount())
    throw LeidenException("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw LeidenException("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->_correct_self_loops = correct_self_loops;
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<Weight> const& edge_weights
  , vector<Id> const& node_sizes): _graph(graph), _remove_graph(false), _owner(nullptr)
{
  if (edge_weights.size() != this->ecount())
    throw LeidenException("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  if (node_sizes.size() != this->vcount())
    throw LeidenException("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->_correct_self_loops = this->has_self_loops();

  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<Weight> const& edge_weights, int correct_self_loops)
  : _graph(graph), _remove_graph(false), _owner(nullptr)
{
  this->_correct_self_loops = correct_self_loops;
  if (edge_weights.size() != this->ecount())
    throw LeidenException("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;
  this->set_default_node_size();
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<Weight> const& edge_weights): _graph(graph)
  , _remove_graph(false), _owner(nullptr)
{
  if (edge_weights.size() != this->ecount())
    throw LeidenException("Edge weights vector inconsistent length with the edge count of the graph.");
  this->_edge_weights = edge_weights;
  this->_is_weighted = true;

  this->_correct_self_loops = this->has_self_loops();

  this->set_default_node_size();
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<Id> const& node_sizes, int correct_self_loops)
  : _graph(graph), _remove_graph(false), _owner(nullptr)
{
  this->_correct_self_loops = correct_self_loops;

  if (node_sizes.size() != this->vcount())
    throw LeidenException("Node size vector inconsistent length with the vertex count of the graph.");
  this->_node_sizes = node_sizes;

  this->set_default_edge_weight();
  this->_is_weighted = false;
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, vector<Id> const& node_sizes): _graph(graph)
  , _remove_graph(false), _owner(nullptr)
{
  this->set_defaults();
  this->_is_weighted = false;

  if (node_sizes.size() != this->vcount())
    throw LeidenException("Node size vector inconsistent length with the vertex count of the graph.");

  this->_node_sizes = node_sizes;

  this->_correct_self_loops = this->has_self_loops();

  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph, int correct_self_loops): _graph(graph)
  , _remove_graph(false), _owner(nullptr)
{
  this->set_defaults();
  this->_correct_self_loops = correct_self_loops;
  this->_is_weighted = false;
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(igraph_t* graph): _graph(graph), _remove_graph(false), _owner(nullptr)
  , _correct_self_loops(has_self_loops())
{
  this->set_defaults();
  this->_is_weighted = false;
  this->init_admin();
  this->set_self_weights();
}

Graph::Graph(): _graph(new igraph_t()), _remove_graph(true), _owner(nullptr)
  , _is_weighted(false), _correct_self_loops(false)
{
  set_defaults();
  init_admin();
  set_self_weights();
}

//Graph::Graph(bool clean) noexcept: _graph(nullptr), _remove_graph(false)
//  , _strength_in(), _strength_out(), _degree_in(), _degree_out(), _degree_all()
//  , _edge_weights(), _node_sizes(), _node_self_weights()
//  , _cached_neighs_from(), _current_node_cache_neigh_from()
//  , _cached_neighs_to(), _current_node_cache_neigh_to()
//  , _cached_neighs_all(), _current_node_cache_neigh_all()
//  , _cached_neigh_edges_from(), _current_node_cache_neigh_edges_from()
//  , _cached_neigh_edges_to(), _current_node_cache_neigh_edges_to()
//  , _cached_neigh_edges_all(), _current_node_cache_neigh_edges_all()
//  , _total_weight(other._total_weight), _total_size(), _is_weighted(), _correct_self_loops(), _density()
//{}

Graph::Graph(igraph_t&& gr) noexcept: _graph(new igraph_t(move(gr))), _remove_graph(true)
  , _owner(nullptr), _is_weighted(igraph_cattribute_has_attr(_graph, IGRAPH_ATTRIBUTE_EDGE, "weight"))
  , _correct_self_loops(has_self_loops())
{
  if(_is_weighted) {
    igraph_vector_t  weights;
    _edge_weights.resize(ecount());
    EANV(_graph, "weight", const_cast<igraph_vector_t*>(
      igraph_vector_view(&weights, _edge_weights.data(), _edge_weights.size())));
    assert(igraph_vector_size(&weights) == _edge_weights.size()
      &&  "_edge_weights number is not synced with the number of edges");
    DELEA(_graph, "weight");
  } else set_default_edge_weight();
//  {
//    // Set default edge weight of 1.0
//    _edge_weights.resize(m);
//    //fill(this->_edge_weights.begin(), this->_edge_weights.end(), 1.0);
//  }
  set_default_node_size();

  init_admin();
  set_self_weights();
}

Graph::Graph(Graph&& other) noexcept: _graph(other._graph), _remove_graph(false), _owner(nullptr)
  , _strength_in(move(other._strength_in)), _strength_out(move(other._strength_out))
  , _degree_in(move(other._degree_in)), _degree_out(move(other._degree_out)), _degree_all(move(other._degree_all))
  , _edge_weights(move(other._edge_weights)), _node_sizes(move(other._node_sizes))
  , _node_self_weights(move(other._node_self_weights))
  , _cached_neighs_from(move(other._cached_neighs_from)), _current_node_cache_neigh_from(other._current_node_cache_neigh_from)
  , _cached_neighs_to(move(other._cached_neighs_to)), _current_node_cache_neigh_to(other._current_node_cache_neigh_to)
  , _cached_neighs_all(move(other._cached_neighs_all)), _current_node_cache_neigh_all(other._current_node_cache_neigh_all)
  , _cached_neigh_edges_from(move(other._cached_neigh_edges_from)), _current_node_cache_neigh_edges_from(other._current_node_cache_neigh_edges_from)
  , _cached_neigh_edges_to(move(other._cached_neigh_edges_to)), _current_node_cache_neigh_edges_to(other._current_node_cache_neigh_edges_to)
  , _cached_neigh_edges_all(move(other._cached_neigh_edges_all)), _current_node_cache_neigh_edges_all(other._current_node_cache_neigh_edges_all)
  , _total_weight(other._total_weight), _total_size(other._total_size), _is_weighted(other._is_weighted)
  , _correct_self_loops(other._correct_self_loops), _density(other._density)
{
  other._remove_graph = false;
  //other._owner = nullptr;  // Note: other's owner should still be capable to release it's memory
  other._graph = nullptr;
  other._is_weighted = false;
  other._correct_self_loops = false;
  //init_admin();
}

Graph::~Graph()
{
  if(_remove_graph && _graph)
  {
    _remove_graph = false;
    // Note: igraph_destroy() includes destruction of the graph attributes if defined any
    igraph_destroy(_graph);
    delete _graph;
    _graph = nullptr;
  }
}

Graph& Graph::operator=(Graph&& other) noexcept
{
  _graph = other._graph;
  other._graph = nullptr;
  _remove_graph = other._remove_graph;
  other._remove_graph = false;
  //other._owner = nullptr;  // Note: other's owner should still be capable to release it's memory

  _edge_weights = move(other._edge_weights);
  _is_weighted = other._is_weighted;
  other._is_weighted = false;

  _node_sizes = move(other._node_sizes);
  _node_self_weights = move(other._node_self_weights);
  _correct_self_loops = other._correct_self_loops;

  //init_admin();
  _strength_in = move(other._strength_in);
  _strength_out = move(other._strength_out);

  _degree_in = move(other._degree_in);
  _degree_out = move(other._degree_out);
  _degree_all = move(other._degree_all);

  _cached_neighs_from = move(other._cached_neighs_from);
  _current_node_cache_neigh_from = other._current_node_cache_neigh_from;
  _cached_neighs_to = move(other._cached_neighs_to);
  _current_node_cache_neigh_to = other._current_node_cache_neigh_to;
  _cached_neighs_all = move(other._cached_neighs_all);
  _current_node_cache_neigh_all = other._current_node_cache_neigh_all;

  _cached_neigh_edges_from = move(other._cached_neigh_edges_from);
  _current_node_cache_neigh_edges_from = other._current_node_cache_neigh_edges_from;
  _cached_neigh_edges_to = move(other._cached_neigh_edges_to);
  _current_node_cache_neigh_edges_to = other._current_node_cache_neigh_edges_to;
  _cached_neigh_edges_all = move(other._cached_neigh_edges_all);
  _current_node_cache_neigh_edges_all = other._current_node_cache_neigh_edges_all;

  _total_weight = other._total_weight;
  _total_size = other._total_size;
  _density = other._density;

  return *this;
}

const MutableVertexPartition* Graph::owner(const MutableVertexPartition* owner) noexcept
{
  if(!_owner || !_owner->get_graph())
    _owner = owner;
  return _owner;
}

int Graph::has_self_loops() const
{
  bool has_self_loops = false;
  if(!_graph)
    return has_self_loops;

  igraph_eit_t  eit;  // Edge iterator
  int err = igraph_eit_create(_graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
  if(err)
    throw LeidenException("Graph::owner() failed in igraph_eit_create(): " + to_string(err));

  while(!IGRAPH_EIT_NEXT(eit)) {
    const auto sid = IGRAPH_EIT_GET(eit);  // Source id
    IGRAPH_EIT_NEXT(eit);
    if(sid == IGRAPH_EIT_GET(eit)) {
      has_self_loops = true;
      break;
    }
    IGRAPH_EIT_NEXT(eit);
  }

  igraph_eit_destroy(&eit);
  return has_self_loops;
}

Id Graph::possible_edges() const noexcept
{
  return this->possible_edges(this->vcount());
}

Id Graph::possible_edges(Id n) const noexcept
{
  Id possible_edges = n*(n-1);
  if (!this->is_directed())
    possible_edges /= 2;
  if (this->correct_self_loops())
    possible_edges += n;

  return possible_edges;
}

void Graph::set_defaults()
{
  this->set_default_edge_weight();
  this->set_default_node_size();
}

void Graph::set_default_edge_weight()
{
  Id m = this->ecount();

  // Set default edge weight of 1.0
  this->_edge_weights.clear(); this->_edge_weights.resize(m);
  fill(this->_edge_weights.begin(), this->_edge_weights.end(), 1.0);
  this->_is_weighted = false;
}

void Graph::set_default_node_size()
{
  Id n = this->vcount();

  // Set default node size of 1
  this->_node_sizes.clear(); this->_node_sizes.resize(n);
  fill(this->_node_sizes.begin(), this->_node_sizes.end(), 1);
}

void Graph::set_self_weights()
{
  Id n = this->vcount();

  // Set default self_weights of the total weight of any possible self-loops
  this->_node_self_weights.clear(); this->_node_self_weights.resize(n);
  for (Id v = 0; v < n; v++)
  {
    #ifdef DEBUG
      cerr << "\t" << "Size node " << v << ": " << this->node_size(v) << endl;
    #endif
    Weight self_weight = 0.0;
    // There should be only one self loop
    igraph_integer_t eid;
    // Get edge id for self loop
    igraph_get_eid(this->_graph, &eid, v, v, this->is_directed(), false);
    if (eid >= 0)
      self_weight = this->edge_weight(eid);

    this->_node_self_weights[v] = self_weight;
    #ifdef DEBUG
      cerr << "\t" << "Self weight node " << v << ": " << self_weight << endl;
    #endif
  }
}

void Graph::init_admin()
{

  const Id m = ecount();

  // Determine total weight in the graph.
  _total_weight = 0.0;
  for (Id e = 0; e < m; e++)
    _total_weight += edge_weight(e);

  // Make sure to multiply by 2 for undirected graphs
  //if (!this->is_directed())
  //  this->_total_weight *= 2.0;

  const Id n = vcount();

  _total_size = 0;
  for (Id v = 0; v < n; v++)
    _total_size += node_size(v);

  igraph_vector_t res;

  // Init weights vector
  igraph_vector_t weights;
  assert(_edge_weights.size() == m && "_edge_weights are not synced with the number of edges");
  igraph_vector_view(&weights, _edge_weights.data(), _edge_weights.size());

  // Strength IN
  //igraph_vector_null(&res);  // Note: igraph_vector_null(&res) is performed automatically
  int err = igraph_vector_init(&res, 0)
    || igraph_strength(_graph, &res, igraph_vss_all(), IGRAPH_IN, true, &weights);
  if(err)
    throw LeidenException("init_admin(), igraph_strength() failed for in: " + to_string(err));
  assert(igraph_vector_size(&res) == n && "Unexpected number of incident vertices with defined strength in");
  _strength_in.assign(igraph_vector_e_ptr(&res, 0), igraph_vector_e_ptr(&res, n));

  // Strength OUT
  // Calculcate strength
  err = igraph_strength(_graph, &res, igraph_vss_all(), IGRAPH_OUT, true, &weights);
  if(err)
    throw LeidenException("init_admin(), igraph_strength() failed for out: " + to_string(err));
  assert(igraph_vector_size(&res) == n && "Unexpected number of edges in strength out");
  _strength_out.assign(igraph_vector_e_ptr(&res, 0), igraph_vector_e_ptr(&res, n));

  // Degree IN
  //igraph_vector_null(&res);  // Note: igraph_vector_null(&res) is performed automatically
  err = igraph_degree(_graph, &res, igraph_vss_all(), IGRAPH_IN, true);
  if(err)
    throw LeidenException("init_admin(), igraph_degree() failed for in: " + to_string(err));
  assert(igraph_vector_size(&res) == n && "Unexpected number of edges in degree in");
  _degree_in.assign(igraph_vector_e_ptr(&res, 0), igraph_vector_e_ptr(&res, n));

  // Degree OUT
  err = igraph_degree(_graph, &res, igraph_vss_all(), IGRAPH_OUT, true);
  if(err)
    throw LeidenException("init_admin(), igraph_degree() failed for out: " + to_string(err));
  assert(igraph_vector_size(&res) == n && "Unexpected number of edges in degree out");
  _degree_out.assign(igraph_vector_e_ptr(&res, 0), igraph_vector_e_ptr(&res, n));

  // Degree ALL
  err = igraph_degree(_graph, &res, igraph_vss_all(), IGRAPH_ALL, true);
  if(err)
    throw LeidenException("init_admin(), igraph_degree() failed for all: " + to_string(err));
  assert(igraph_vector_size(&res) == n && "Unexpected number of edges in degree all");
  _degree_all.assign(igraph_vector_e_ptr(&res, 0), igraph_vector_e_ptr(&res, n));

  igraph_vector_destroy(&res);

  // Calculate density;
  Weight w = total_weight();
  Id n_size = total_size();

  // For now we default to not correcting self loops.
  // this->_correct_self_loops = false; (remove this as this is set in the constructor)

  Weight normalise = 0.0;
  if (this->_correct_self_loops)
    normalise = n_size*n_size;
  else
    normalise = n_size*(n_size - 1);

  if (this->is_directed())
    this->_density = w/normalise;
  else
    this->_density = 2*w/normalise;

  this->_current_node_cache_neigh_edges_from = n + 1;
  this->_current_node_cache_neigh_edges_to = n + 1;
  this->_current_node_cache_neigh_edges_all = n + 1;

  this->_current_node_cache_neigh_from = n + 1;
  this->_current_node_cache_neigh_to = n + 1;
  this->_current_node_cache_neigh_all = n + 1;
}

void Graph::cache_neighbour_edges(Id v, igraph_neimode_t mode) const noexcept
{
  #ifdef DEBUG
    cerr << "void Graph::cache_neighbour_edges(" << v << ", " << mode << ");" << endl;
  #endif

  vector<Id>* _cached_neigh_edges = nullptr;
  switch (mode)
  {
    case IGRAPH_IN:
      this->_current_node_cache_neigh_edges_from = v;
      _cached_neigh_edges = &_cached_neigh_edges_from;
      break;
    case IGRAPH_OUT:
      this->_current_node_cache_neigh_edges_to = v;
      _cached_neigh_edges = &_cached_neigh_edges_to;
      break;
    case IGRAPH_ALL:
    default:
      this->_current_node_cache_neigh_edges_all = v;
      _cached_neigh_edges = &_cached_neigh_edges_all;
  }
  igraph_vector_t incident_edges;
  int err = igraph_vector_init(&incident_edges, 0)
    || igraph_incident(_graph, &incident_edges, v, mode);
  if(err)
    throw LeidenException("cache_neighbour_edges(), incident_edges failed: " + to_string(err));
  assert(degree(v, mode) == igraph_vector_size(&incident_edges)
    && "cache_neighbour_edges(), incident_edges are not synchronized with the vertex degree");
  _cached_neigh_edges->assign(igraph_vector_e_ptr(&incident_edges, 0)
    , igraph_vector_e_ptr(&incident_edges, igraph_vector_size(&incident_edges)));
  igraph_vector_destroy(&incident_edges);

  #ifdef DEBUG
    cerr << "Degree: " << degree(v, mode) << endl;
    cerr << "Number of edges: " << _cached_neigh_edges->size() << endl;
    cerr << "exit void Graph::cache_neighbour_edges(" << v << ", " << mode << ");" << endl;
  #endif
}

vector<Id> const& Graph::get_neighbour_edges(Id v, igraph_neimode_t mode) const noexcept
{
  switch (mode)
  {
    case IGRAPH_IN:
      if (this->_current_node_cache_neigh_edges_from != v)
      {
        cache_neighbour_edges(v, mode);
        this->_current_node_cache_neigh_edges_from = v;
      }
      return this->_cached_neigh_edges_from;
    case IGRAPH_OUT:
      if (this->_current_node_cache_neigh_edges_to != v)
      {
        cache_neighbour_edges(v, mode);
        this->_current_node_cache_neigh_edges_to = v;
      }
      return this->_cached_neigh_edges_to;
    case IGRAPH_ALL:
      if (this->_current_node_cache_neigh_edges_all != v)
      {
        cache_neighbour_edges(v, mode);
        this->_current_node_cache_neigh_edges_all = v;
      }
      return this->_cached_neigh_edges_all;
    default:
      throw LeidenException("Incorrect model for getting neighbour edges.");
  }
}

pair<Id, Id> Graph::get_endpoints(Id e) const noexcept
{
  igraph_integer_t from, to;
  igraph_edge(this->_graph, e,&from, &to);
  return make_pair<Id, Id>((Id)from, (Id)to);
}

void Graph::cache_neighbours(Id v, igraph_neimode_t mode) const noexcept
{
  #ifdef DEBUG
    cerr << "void Graph::cache_neighbours(" << v << ", " << mode << ");" << endl;
  #endif

  vector<Id>* _cached_neighs = nullptr;
  switch (mode)
  {
    case IGRAPH_IN:
      this->_current_node_cache_neigh_from = v;
      _cached_neighs = &(this->_cached_neighs_from);
      break;
    case IGRAPH_OUT:
      this->_current_node_cache_neigh_to = v;
      _cached_neighs = &(this->_cached_neighs_to);
      break;
    case IGRAPH_ALL:
    default:
      this->_current_node_cache_neigh_all = v;
      _cached_neighs = &(this->_cached_neighs_all);
  }
  igraph_vector_t neighbours;
  int err = igraph_vector_init(&neighbours, 0)
    || igraph_neighbors(_graph, &neighbours, v, mode);
  if(err)
    throw LeidenException("get_endpoints(), neighbours failed: " + to_string(err));
  assert(degree(v, mode) == igraph_vector_size(&neighbours)
    && "cache_neighbours(), neighbours are not synchronized with the vertex degree");
  _cached_neighs->assign(igraph_vector_e_ptr(&neighbours, 0)
    , igraph_vector_e_ptr(&neighbours, igraph_vector_size(&neighbours)));
  igraph_vector_destroy(&neighbours);

  #ifdef DEBUG
    cerr << "Degree: " << >degree(v, mode) << endl;
    cerr << "Number of edges: " << _cached_neighs->size() << endl;
    cerr << "exit void Graph::cache_neighbours(" << v << ", " << mode << ");" << endl;
  #endif
}

vector< Id > const& Graph::get_neighbours(Id v, igraph_neimode_t mode) const
{
  switch (mode)
  {
    case IGRAPH_IN:
      if (this->_current_node_cache_neigh_from != v)
      {
        cache_neighbours(v, mode);
        this -> _current_node_cache_neigh_from = v;
      }
      #ifdef DEBUG
        cerr << "Returning " << this->_cached_neighs_from.size() << " incoming neighbours" << endl;
      #endif
      return this->_cached_neighs_from;
    case IGRAPH_OUT:
      if (this->_current_node_cache_neigh_to != v)
      {
        cache_neighbours(v, mode);
        this -> _current_node_cache_neigh_to = v;
      }
      #ifdef DEBUG
        cerr << "Returning " << this->_cached_neighs_to.size() << " incoming neighbours" << endl;
      #endif
      return this->_cached_neighs_to;
    case IGRAPH_ALL:
      if (this->_current_node_cache_neigh_all != v)
      {
        cache_neighbours(v, mode);
        this->_current_node_cache_neigh_all = v;
      }
      #ifdef DEBUG
        cerr << "Returning " << this->_cached_neighs_all.size() << " incoming neighbours" << endl;
      #endif
      return this->_cached_neighs_all;
    default:
      throw LeidenException("Invalid mode for getting neighbors.");
  }
}

/********************************************************************************
 * This should return a random neighbour in O(1)
 ********************************************************************************/
Id Graph::get_random_neighbour(Id v, igraph_neimode_t mode, igraph_rng_t* rng) const
{
  Id node=v;
  Id rand_neigh = -1;

  if (this->degree(v, mode) <= 0)
    throw LeidenException("Cannot select a random neighbour for an isolated node.");

  if (igraph_is_directed(this->_graph) && mode != IGRAPH_ALL)
  {
    if (mode == IGRAPH_OUT)
    {
      // Get indices of where neighbours are
      Id cum_degree_this_node = (Id) VECTOR(this->_graph->os)[node];
      Id cum_degree_next_node = (Id) VECTOR(this->_graph->os)[node+1];
      // Get a random index from them
      Id rand_neigh_idx = get_random_int(cum_degree_this_node, cum_degree_next_node - 1, rng);
      // Return the neighbour at that index
      #ifdef DEBUG
        cerr << "Degree: " << this->degree(node, mode) << " diff in cumulative: " << cum_degree_next_node - cum_degree_this_node << endl;
      #endif
      rand_neigh = VECTOR(this->_graph->to)[ (Id)VECTOR(this->_graph->oi)[rand_neigh_idx] ];
    }
    else if (mode == IGRAPH_IN)
    {
      // Get indices of where neighbours are
      Id cum_degree_this_node = (Id) VECTOR(this->_graph->is)[node];
      Id cum_degree_next_node = (Id) VECTOR(this->_graph->is)[node+1];
      // Get a random index from them
      Id rand_neigh_idx = get_random_int(cum_degree_this_node, cum_degree_next_node - 1, rng);
      #ifdef DEBUG
        cerr << "Degree: " << this->degree(node, mode) << " diff in cumulative: " << cum_degree_next_node - cum_degree_this_node << endl;
      #endif
      // Return the neighbour at that index
      rand_neigh = VECTOR(this->_graph->from)[ (Id)VECTOR(this->_graph->ii)[rand_neigh_idx] ];
    }
  }
  else
  {
    // both in- and out- neighbors in a directed graph.
    Id cum_outdegree_this_node = (Id)VECTOR(this->_graph->os)[node];
    Id cum_indegree_this_node  = (Id)VECTOR(this->_graph->is)[node];

    Id cum_outdegree_next_node = (Id)VECTOR(this->_graph->os)[node+1];
    Id cum_indegree_next_node  = (Id)VECTOR(this->_graph->is)[node+1];

    Id total_outdegree = cum_outdegree_next_node - cum_outdegree_this_node;
    Id total_indegree = cum_indegree_next_node - cum_indegree_this_node;

    Id rand_idx = get_random_int(0, total_outdegree + total_indegree - 1, rng);

    #ifdef DEBUG
      cerr << "Degree: " << this->degree(node, mode) << " diff in cumulative: " << total_outdegree + total_indegree << endl;
    #endif
    // From among in or out neighbours?
    if (rand_idx < total_outdegree)
    { // From among outgoing neighbours
      Id rand_neigh_idx = cum_outdegree_this_node + rand_idx;
      rand_neigh = VECTOR(this->_graph->to)[ (Id)VECTOR(this->_graph->oi)[rand_neigh_idx] ];
    }
    else
    { // From among incoming neighbours
      Id rand_neigh_idx = cum_indegree_this_node + rand_idx - total_outdegree;
      rand_neigh = VECTOR(this->_graph->from)[ (Id)VECTOR(this->_graph->ii)[rand_neigh_idx] ];
    }
  }

  return rand_neigh;
}

/****************************************************************************
  Creates a graph with communities as node and links as weights between
  communities.

  The weight of the edges in the new graph is simply the sum of the weight
  of the edges between the communities. The self weight of a node (i.e. the
  weight of its self loop) is the internal weight of a community. The size
  of a node in the new graph is simply the size of the community in the old
  graph.
*****************************************************************************/
Graph* Graph::collapse_graph(MutableVertexPartition* partition) const
{
  #ifdef DEBUG
    cerr << "Graph* Graph::collapse_graph(vector<Id> membership)" << endl;
  #endif
  Id m = this->ecount();

  #ifdef DEBUG
    cerr << "Current graph has " << this->vcount() << " nodes and " << this->ecount() << " edges." << endl;
    cerr << "Collapsing to graph with " << partition->n_communities() << " nodes." << endl;
  #endif

  vector< map<Id, Weight> > collapsed_edge_weights(partition->n_communities());

  igraph_integer_t v, u;
  for (Id e = 0; e < m; e++)
  {
    Weight w = this->edge_weight(e);
    igraph_edge(_graph, e, &v, &u);
    Id v_comm = partition->membership((Id)v);
    Id u_comm = partition->membership((Id)u);
    if (collapsed_edge_weights[v_comm].count(u_comm) > 0)
      collapsed_edge_weights[v_comm][u_comm] += w;
    else
      collapsed_edge_weights[v_comm][u_comm] = w;
  }

  // Now create vector for edges, first determined the number of edges
  Id m_collapsed = 0;
  Id n_collapsed = partition->n_communities();

  for (vector< map<Id, Weight> >::iterator itr = collapsed_edge_weights.begin();
       itr != collapsed_edge_weights.end(); itr++)
  {
      m_collapsed += itr->size();
  }

  igraph_vector_t edges;
  vector<Weight> collapsed_weights(m_collapsed, 0.0);
  Weight total_collapsed_weight = 0.0;

  int err = igraph_vector_init(&edges, 2*m_collapsed); // Vector or edges with edges (edge[0], edge[1]), (edge[2], edge[3]), etc...
  if(err)
    throw LeidenException("collapse_graph(), igraph_vector_init() failed: " + to_string(err));

  Id e_idx = 0;
  for (Id v = 0; v < n_collapsed; v++)
  {
    for (map<Id, Weight>::iterator itr = collapsed_edge_weights[v].begin();
         itr != collapsed_edge_weights[v].end(); itr++)
    {
      if (e_idx >= m_collapsed)
        throw LeidenException("Maximum number of possible edges exceeded.");

      Id u = itr->first;
      Weight w = itr->second;
      VECTOR(edges)[2*e_idx] = v;
      VECTOR(edges)[2*e_idx+1] = u;
      collapsed_weights[e_idx] = w;
      total_collapsed_weight += w;
      // next edge
      e_idx += 1;
    }
  }

  // Create graph based on edges
  igraph_t* graph = new igraph_t();
  err = igraph_create(graph, &edges, n_collapsed, this->is_directed());
  if(err)
    throw LeidenException("collapse_graph(), igraph_create() failed: " + to_string(err));
  igraph_vector_destroy(&edges);

  if ((Id) igraph_vcount(graph) != partition->n_communities())
    throw LeidenException("Something went wrong with collapsing the graph.");

  // Calculate new node sizes
  vector<Id> csizes(n_collapsed, 0);
  for (Id c = 0; c < partition->n_communities(); c++)
    csizes[c] = partition->csize(c);

  Graph* G = new Graph(graph, collapsed_weights, csizes, this->_correct_self_loops);
  G->_remove_graph = true;
  #ifdef DEBUG
    cerr << "exit Graph::collapse_graph(vector<Id> membership)" << endl << endl;
  #endif
  return G;
}
