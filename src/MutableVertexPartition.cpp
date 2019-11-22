#include "MutableVertexPartition.h"

#ifdef DEBUG
  using std::cerr;
  using std::endl;
#endif

/****************************************************************************
  Create a new vertex partition.

  Parameters:
    graph            -- The igraph.Graph on which this partition is defined.
    membership=None  -- The membership vector of this partition, i.e. an
                        community number for each node. So membership[i] = c
                        implies that node i is in community c. If None, it is
                        initialised with each node in its own community.
    weight_attr=None -- What edge attribute should be used as a weight for the
                        edges? If None, the weight defaults to 1.
    size_attr=None   -- What node attribute should be used for keeping track
                        of the size of the node? In some methods (e.g. CPM or
                        Significance), we need to keep track of the total
                        size of the community. So when we aggregate/collapse
                        the graph, we should know how many nodes were in a
                        community. If None, the size of a node defaults to 1.
    self_weight_attr=None
                     -- What node attribute should be used for the self
                        weight? If None, the self_weight is
                        recalculated each time."""
*****************************************************************************/
MutableVertexPartition::MutableVertexPartition(const Graph* graph, vector<Id> const& membership)
: graph(graph), destructor_delete_graph(false), _membership(membership)
{
  if (membership.size() != graph->vcount())
    throw LeidenException("Membership vector has incorrect size.");
  init_admin();
}

MutableVertexPartition* MutableVertexPartition::create(const Graph* graph, vector<Id> const& membership)
{
  return new MutableVertexPartition(graph, membership);
}

MutableVertexPartition::MutableVertexPartition(const Graph* graph)
: graph(graph), destructor_delete_graph(false),_membership(range(graph->vcount()))
{
  init_admin();
}

MutableVertexPartition* MutableVertexPartition::create(const Graph* graph)
{
  return new MutableVertexPartition(graph);
}

MutableVertexPartition::MutableVertexPartition(MutableVertexPartition&& other) noexcept: graph(other.graph)
  , destructor_delete_graph(other.destructor_delete_graph), _membership(other._membership)
{
  other.graph = nullptr;
  other.destructor_delete_graph = false;
}

MutableVertexPartition::~MutableVertexPartition()
{
  this->clean_mem();
  if (destructor_delete_graph && graph) {
    delete graph;
    graph = nullptr;
  }
}

MutableVertexPartition& MutableVertexPartition::operator=(MutableVertexPartition&& other) noexcept
{
  graph = other.graph;
  other.graph = nullptr;

  destructor_delete_graph = other.destructor_delete_graph;
  other.destructor_delete_graph = false;

  _membership = move(other._membership);
}

void MutableVertexPartition::clean_mem()
{}

Id MutableVertexPartition::csize(Id comm) const noexcept
{
  if (comm < this->_csize.size())
    return this->_csize[comm];
  else
    return 0;
}

Id MutableVertexPartition::cnodes(Id comm) const noexcept
{
  if (comm < this->_cnodes.size())
    return this->_cnodes[comm];
  else
    return 0;
}

vector<Id> MutableVertexPartition::get_community(Id comm) const noexcept
{
  vector<Id> community;
  community.reserve(this->_cnodes[comm]);
  for (Id i = 0; i < this->graph->vcount(); i++)
    if (this->_membership[i] == comm)
      community.push_back(i);
  return community;
}

vector< vector<Id> > MutableVertexPartition::get_communities() const noexcept
{
  vector< vector<Id> > communities(this->_n_communities);

  for (Id c = 0; c < this->_n_communities; c++)
  {
    Id cn = this->_cnodes[c];
    communities[c].reserve(cn);
  }

  for (Id i = 0; i < this->graph->vcount(); i++)
      communities[this->_membership[i]].push_back(i);

  return communities;
}

Id MutableVertexPartition::n_communities() const noexcept
{
  return this->_n_communities;
}

/****************************************************************************
  Initialise all the administration based on the membership vector.
*****************************************************************************/

void MutableVertexPartition::init_admin()
{
  #ifdef DEBUG
    cerr << "void MutableVertexPartition::init_admin()" << endl;
  #endif
  Id n = this->graph->vcount();

  // First determine number of communities (assuming they are consecutively numbered
  this->update_n_communities();

  // Reset administration
  this->_total_weight_in_comm.clear();
  this->_total_weight_in_comm.resize(this->_n_communities);
  this->_total_weight_from_comm.clear();
  this->_total_weight_from_comm.resize(this->_n_communities);
  this->_total_weight_to_comm.clear();
  this->_total_weight_to_comm.resize(this->_n_communities);
  this->_csize.clear();
  this->_csize.resize(this->_n_communities);
  this->_cnodes.clear();
  this->_cnodes.resize(this->_n_communities);

  this->_current_node_cache_community_from = n + 1; this->_cached_weight_from_community.resize(this->_n_communities, 0);
  this->_current_node_cache_community_to = n + 1;   this->_cached_weight_to_community.resize(this->_n_communities, 0);
  this->_current_node_cache_community_all = n + 1;  this->_cached_weight_all_community.resize(this->_n_communities, 0);

  this->_empty_communities.clear();

  this->_total_weight_in_all_comms = 0.0;
  for (Id v = 0; v < n; v++)
  {
    Id v_comm = this->_membership[v];
    // Update the community size
    this->_csize[v_comm] += this->graph->node_size(v);
    // Update the community size
    this->_cnodes[v_comm] += 1;
  }

  Id m = graph->ecount();
  for (Id e = 0; e < m; e++)
  {
    pair<Id, Id> endpoints = this->graph->get_endpoints(e);
    Id v = endpoints.first;
    Id u = endpoints.second;

    Id v_comm = this->_membership[v];
    Id u_comm = this->_membership[u];

    // Get the weight of the edge
    Weight w = this->graph->edge_weight(e);
    // Add weight to the outgoing weight of community of v
    this->_total_weight_from_comm[v_comm] += w;
    #ifdef DEBUG
      cerr << "\t" << "Add (" << v << ", " << u << ") weight " << w << " to from_comm " << v_comm <<  "." << endl;
    #endif
    // Add weight to the incoming weight of community of u
    this->_total_weight_to_comm[u_comm] += w;
    #ifdef DEBUG
      cerr << "\t" << "Add (" << v << ", " << u << ") weight " << w << " to to_comm " << u_comm << "." << endl;
    #endif
    if (!this->graph->is_directed())
    {
      #ifdef DEBUG
        cerr << "\t" << "Add (" << u << ", " << v << ") weight " << w << " to from_comm " << u_comm <<  "." << endl;
      #endif
      this->_total_weight_from_comm[u_comm] += w;
      #ifdef DEBUG
        cerr << "\t" << "Add (" << u << ", " << v << ") weight " << w << " to to_comm " << v_comm << "." << endl;
      #endif
      this->_total_weight_to_comm[v_comm] += w;
    }
    // If it is an edge within a community
    if (v_comm == u_comm)
    {
      this->_total_weight_in_comm[v_comm] += w;
      this->_total_weight_in_all_comms += w;
      #ifdef DEBUG
        cerr << "\t" << "Add (" << v << ", " << u << ") weight " << w << " to in_comm " << v_comm << "." << endl;
      #endif
    }
  }

  this->_total_possible_edges_in_all_comms = 0;
  for (Id c = 0; c < this->_n_communities; c++)
  {
    Id n_c = this->csize(c);
    Id possible_edges = this->graph->possible_edges(n_c);

    #ifdef DEBUG
      cerr << "\t" << "c=" << c << ", n_c=" << n_c << ", possible_edges=" << possible_edges << endl;
    #endif

    this->_total_possible_edges_in_all_comms += possible_edges;

    // It is possible that some community have a zero size (if the order
    // is for example not consecutive. We add those communities to the empty
    // communities vector for consistency.
    if (this->_cnodes[c] == 0)
      this->_empty_communities.push_back(c);
  }

  #ifdef DEBUG
    cerr << "exit MutableVertexPartition::init_admin()" << endl << endl;
  #endif

}

void MutableVertexPartition::update_n_communities()
{
  this->_n_communities = 0;
  for (Id i = 0; i < this->graph->vcount(); i++)
    if (this->_membership[i] >= this->_n_communities)
      this->_n_communities = this->_membership[i] + 1;
}

/****************************************************************************
 Renumber the communities so that they are numbered 0,...,q-1 where q is
 the number of communities. This also removes any empty communities, as they
 will not be given a new number.
*****************************************************************************/
void MutableVertexPartition::renumber_communities()
{
  vector<MutableVertexPartition*> partitions(1);
  partitions[0] = this;
  this->renumber_communities(MutableVertexPartition::renumber_communities(partitions));
}

vector<Id> MutableVertexPartition::renumber_communities(vector<MutableVertexPartition*> partitions)
{
  Id nb_layers = partitions.size();
  Id nb_comms = partitions[0]->n_communities();
  Id n = partitions[0]->graph->vcount();

  #ifdef DEBUG
    for (Id layer; layer < nb_layers; layer++)
    {
      for (Id v = 0; v < n; v++)
      {
        if (partitions[0]->membership(v) != partitions[layer]->membership(v))
          cerr << "Membership of all partitions are not equal";
      }
    }
  #endif
  // First sort the communities by size
  // Csizes
  // first - community
  // second - csize
  // third - number of nodes (may be aggregate nodes), to account for communities with zero weight.
  vector<Id*> csizes;
  for (Id i = 0; i < nb_comms; i++)
  {
      Id csize = 0;
      for (Id layer = 0; layer < nb_layers; layer++)
        csize += partitions[layer]->csize(i);

      Id* row = new Id[3];
      row[0] = i;
      row[1] = csize;
      row[2] = partitions[0]->cnodes(i);
      csizes.push_back(row);
  }
  sort(csizes.begin(), csizes.end(), orderCSize);

  // Then use the sort order to assign new communities,
  // such that the largest community gets the lowest index.
  vector<Id> new_comm_id(nb_comms, 0);
  for (Id i = 0; i < nb_comms; i++)
  {
    Id comm = csizes[i][0];
    new_comm_id[comm] = i;
    delete[] csizes[i];
  }

  vector<Id> membership(n, 0);
  for (Id i = 0; i < n; i++)
    membership[i] = new_comm_id[partitions[0]->_membership[i]];

  return membership;
}


/****************************************************************************
 Renumber the communities using the provided membership vector. Notice that this
 doesn't ensure any property of the community numbers.
*****************************************************************************/
void MutableVertexPartition::renumber_communities(vector<Id> const& membership)
{
  this->set_membership(membership);
}

Id MutableVertexPartition::get_empty_community()
{
  if (this->_empty_communities.empty())
  {
    // If there was no empty community yet,
    // we will create a new one.
    add_empty_community();
  }

  return this->_empty_communities.back();
}

void MutableVertexPartition::set_membership(vector<Id> const& membership)
{
  #ifdef DEBUG
    cerr << "void MutableVertexPartition::set_membership(" << &membership << ")" << endl;
  #endif
  for (Id i = 0; i < this->graph->vcount(); i++)
  {
    this->_membership[i] = membership[i];
    #ifdef DEBUG
      cerr << "Setting membership[" << i << "] = " << membership[i] << "." << endl;
    #endif
  }

  this->clean_mem();
  this->init_admin();
  #ifdef DEBUG
    cerr << "exit MutableVertexPartition::set_membership(" << &membership << ")" << endl;
  #endif
}

Id MutableVertexPartition::add_empty_community()
{
  this->_n_communities = this->_n_communities + 1;

  if (this->_n_communities > this->graph->vcount())
    throw LeidenException("There cannot be more communities than nodes, so there must already be an empty community.");

  Id new_comm = this->_n_communities - 1;

  this->_csize.resize(this->_n_communities);                  this->_csize[new_comm] = 0;
  this->_cnodes.resize(this->_n_communities);                 this->_cnodes[new_comm] = 0;
  this->_total_weight_in_comm.resize(this->_n_communities);   this->_total_weight_in_comm[new_comm] = 0;
  this->_total_weight_from_comm.resize(this->_n_communities); this->_total_weight_from_comm[new_comm] = 0;
  this->_total_weight_to_comm.resize(this->_n_communities);   this->_total_weight_to_comm[new_comm] = 0;

  this->_cached_weight_all_community.resize(this->_n_communities);
  this->_cached_weight_from_community.resize(this->_n_communities);
  this->_cached_weight_to_community.resize(this->_n_communities);

  this->_empty_communities.push_back(new_comm);
  #ifdef DEBUG
    cerr << "Added empty community " << new_comm << endl;
  #endif
  return new_comm;
}

/****************************************************************************
  Move a node to a new community and update the administration.
  Parameters:
    v        -- Node to move.
    new_comm -- To which community should it move.
*****************************************************************************/
void MutableVertexPartition::move_node(Id v,Id new_comm)
{
  #ifdef DEBUG
    cerr << "void MutableVertexPartition::move_node(" << v << ", " << new_comm << ")" << endl;
    if (new_comm >= this->n_communities())
      cerr << "ERROR: New community (" << new_comm << ") larger than total number of communities (" << this->n_communities() << ")." << endl;
  #endif
  // Move node and update internal administration
  if (new_comm >= this->_n_communities)
  {
    if (new_comm < this->graph->vcount())
    {
      while (new_comm >= this->_n_communities)
        this->add_empty_community();
    }
    else
    {
      throw LeidenException("Cannot add new communities beyond the number of nodes.");
    }
  }

  // Keep track of all possible edges in all communities;
  Id node_size = this->graph->node_size(v);
  Id old_comm = this->_membership[v];
  #ifdef DEBUG
    cerr << "Node size: " << node_size << ", old comm: " << old_comm << ", new comm: " << new_comm << endl;
  #endif
  // Incidentally, this is independent of whether we take into account self-loops or not
  // (i.e. whether we count as n_c^2 or as n_c(n_c - 1). Be careful to do this before the
  // adaptation of the community sizes, otherwise the calculations are incorrect.
  if (new_comm != old_comm)
  {
    Weight delta_possible_edges_in_comms = 2.0*node_size*(ptrdiff_t)(this->_csize[new_comm] - this->_csize[old_comm] + node_size)/(2.0 - this->graph->is_directed());
    _total_possible_edges_in_all_comms += delta_possible_edges_in_comms;
    #ifdef DEBUG
      cerr << "Change in possible edges in all comms: " << delta_possible_edges_in_comms << endl;
    #endif
  }

  // Remove from old community
  #ifdef DEBUG
    cerr << "Removing from old community " << old_comm << ", community size: " << this->_csize[old_comm] << endl;
  #endif
  this->_cnodes[old_comm] -= 1;
  this->_csize[old_comm] -= node_size;
  #ifdef DEBUG
    cerr << "Removed from old community." << endl;
  #endif

  // We have to use the size of the set of nodes rather than the csize
  // to account for nodes that have a zero size (i.e. community may not be empty, but
  // may have zero size).
  if (this->_cnodes[old_comm] == 0)
  {
    #ifdef DEBUG
      cerr << "Adding community " << old_comm << " to empty communities." << endl;
    #endif
    this->_empty_communities.push_back(old_comm);
    #ifdef DEBUG
      cerr << "Added community " << old_comm << " to empty communities." << endl;
    #endif
  }

  if (this->_cnodes[new_comm] == 0)
  {
    #ifdef DEBUG
      cerr << "Removing from empty communities (number of empty communities is " << this->_empty_communities.size() << ")." << endl;
    #endif
    vector<Id>::reverse_iterator it_comm = this->_empty_communities.rbegin();
    while (it_comm != this->_empty_communities.rend() && *it_comm != new_comm)
    {
      #ifdef DEBUG
        cerr << "Empty community " << *it_comm << " != new community " << new_comm << endl;
      #endif
      it_comm++;
    }
    #ifdef DEBUG
      cerr << "Erasing empty community " << *it_comm << endl;
      if (it_comm == this->_empty_communities.rend())
        cerr << "ERROR: empty community does not exist." << endl;
    #endif
    if (it_comm != this->_empty_communities.rend())
      this->_empty_communities.erase( (++it_comm).base() );
  }

  #ifdef DEBUG
    cerr << "Adding to new community " << new_comm << ", community size: " << this->_csize[new_comm] << endl;
  #endif
  // Add to new community
  this->_cnodes[new_comm] += 1;
  this->_csize[new_comm] += this->graph->node_size(v);

  // Switch outgoing links
  #ifdef DEBUG
    cerr << "Added to new community." << endl;
  #endif

  // Use set for incident edges, because self loop appears twice
  igraph_neimode_t modes[2] = {IGRAPH_OUT, IGRAPH_IN};
  for (Id mode_i = 0; mode_i < 2; mode_i++)
  {
    igraph_neimode_t mode = modes[mode_i];

    // Loop over all incident edges
    vector<Id> const& neighbours = this->graph->get_neighbours(v, mode);
    vector<Id> const& neighbour_edges = this->graph->get_neighbour_edges(v, mode);

    Id degree = neighbours.size();

    #ifdef DEBUG
      if (mode == IGRAPH_OUT)
        cerr << "\t" << "Looping over outgoing links." << endl;
      else if (mode == IGRAPH_IN)
        cerr << "\t" << "Looping over incoming links." << endl;
      else
        cerr << "\t" << "Looping over unknown mode." << endl;
    #endif

    for (Id idx = 0; idx < degree; idx++)
    {
      Id u = neighbours[idx];
      Id e = neighbour_edges[idx];

      Id u_comm = this->_membership[u];
      // Get the weight of the edge
      Weight w = this->graph->edge_weight(e);
      if (mode == IGRAPH_OUT)
      {
        // Remove the weight from the outgoing weights of the old community
        this->_total_weight_from_comm[old_comm] -= w;
        // Add the weight to the outgoing weights of the new community
        this->_total_weight_from_comm[new_comm] += w;
        #ifdef DEBUG
          cerr << "\t" << "Moving link (" << v << "-" << u << ") "
               << "outgoing weight " << w
               << " from " << old_comm << " to " << new_comm
               << "." << endl;
        #endif
      }
      else if (mode == IGRAPH_IN)
      {
        // Remove the weight from the outgoing weights of the old community
        this->_total_weight_to_comm[old_comm] -= w;
        // Add the weight to the outgoing weights of the new community
        this->_total_weight_to_comm[new_comm] += w;
        #ifdef DEBUG
          cerr << "\t" << "Moving link (" << v << "-" << u << ") "
               << "incoming weight " << w
               << " from " << old_comm << " to " << new_comm
               << "." << endl;
        #endif
      }
      else
        throw LeidenException("Incorrect mode for updating the admin.");
      // Get internal weight (if it is an internal edge)
      Weight int_weight = w/(this->graph->is_directed() ? 1.0 : 2.0)/( u == v ? 2.0 : 1.0);
      // If it is an internal edge in the old community
      if (old_comm == u_comm)
      {
        // Remove the internal weight
        this->_total_weight_in_comm[old_comm] -= int_weight;
        this->_total_weight_in_all_comms -= int_weight;
        #ifdef DEBUG
          cerr << "\t" << "From link (" << v << "-" << u << ") "
               << "remove internal weight " << int_weight
               << " from " << old_comm << "." << endl;
        #endif
      }
      // If it is an internal edge in the new community
      // i.e. if u is in the new community, or if it is a self loop
      if ((new_comm == u_comm) || (u == v))
      {
        // Add the internal weight
        this->_total_weight_in_comm[new_comm] += int_weight;
        this->_total_weight_in_all_comms += int_weight;
        #ifdef DEBUG
          cerr << "\t" << "From link (" << v << "-" << u << ") "
               << "add internal weight " << int_weight
               << " to " << new_comm << "." << endl;
        #endif
      }
    }
  }
  #ifdef DEBUG
    // Check this->_total_weight_in_all_comms
    Weight check_total_weight_in_all_comms = 0.0;
    for (Id c = 0; c < this->n_communities(); c++)
      check_total_weight_in_all_comms += this->total_weight_in_comm(c);
    cerr << "Internal _total_weight_in_all_comms=" << this->_total_weight_in_all_comms
         << ", calculated check_total_weight_in_all_comms=" << check_total_weight_in_all_comms << endl;
  #endif
  // Update the membership vector
  this->_membership[v] = new_comm;
  #ifdef DEBUG
    cerr << "exit MutableVertexPartition::move_node(" << v << ", " << new_comm << ")" << endl << endl;
  #endif
}


/****************************************************************************
 Read new communities from coarser partition assuming that the community
 represents a node in the coarser partition (with the same index as the
 community number).
****************************************************************************/
void MutableVertexPartition::from_coarse_partition(vector<Id> const& coarse_partition_membership)
{
  this->from_coarse_partition(coarse_partition_membership, this->_membership);
}

void MutableVertexPartition::from_coarse_partition(MutableVertexPartition* coarse_partition)
{
  this->from_coarse_partition(coarse_partition, this->_membership);
}

void MutableVertexPartition::from_coarse_partition(MutableVertexPartition* coarse_partition, vector<Id> const& coarse_node)
{
  this->from_coarse_partition(coarse_partition->membership(), coarse_node);
}

/****************************************************************************
 Set the current community of all nodes to the community specified in the partition
 assuming that the coarser partition is created using the membership as specified
 by coarser_membership. In other words node i becomes node coarse_node[i] in
 the coarser partition and thus has community coarse_partition_membership[coarse_node[i]].
****************************************************************************/
void MutableVertexPartition::from_coarse_partition(vector<Id> const& coarse_partition_membership, vector<Id> const& coarse_node)
{
  // Read the coarser partition
  for (Id v = 0; v < this->graph->vcount(); v++)
  {
    // In the coarser partition, the node should have the community id
    // as represented by the coarser_membership vector
    Id v_level2 = coarse_node[v];

    // In the coarser partition, this node is represented by v_level2
    Id v_comm_level2 = coarse_partition_membership[v_level2];

    // Set local membership to community found for node at second level
    this->_membership[v] = v_comm_level2;
  }

  this->clean_mem();
  this->init_admin();
}


/****************************************************************************
 Read new partition from another partition.
****************************************************************************/
void MutableVertexPartition::from_partition(MutableVertexPartition* partition)
{
  // Assign the membership of every node in the supplied partition
  // to the one in this partition
  for (Id v = 0; v < this->graph->vcount(); v++)
    this->_membership[v] = partition->membership(v);
  this->clean_mem();
  this->init_admin();
}

/****************************************************************************
 Calculate what is the total weight going from a node to a community.

    Parameters:
      v      -- The node which to check.
      comm   -- The community which to check.
*****************************************************************************/
Weight MutableVertexPartition::weight_to_comm(Id v, Id comm) const noexcept
{
  if (this->_current_node_cache_community_to != v)
  {
    this->cache_neigh_communities(v, IGRAPH_OUT);
    this->_current_node_cache_community_to = v;
  }

  if (comm < this->_cached_weight_to_community.size())
    return this->_cached_weight_to_community[comm];
  else
    return 0;
}

/****************************************************************************
 Calculate what is the total weight going from a community to a node.

    Parameters:
      v      -- The node which to check.
      comm   -- The community which to check.
*****************************************************************************/
Weight MutableVertexPartition::weight_from_comm(Id v, Id comm) const noexcept
{
  if (this->_current_node_cache_community_from != v)
  {
    this->cache_neigh_communities(v, IGRAPH_IN);
    this->_current_node_cache_community_from = v;
  }

  if (comm < this->_cached_weight_from_community.size())
    return this->_cached_weight_from_community[comm];
  else
    return 0;
}

void MutableVertexPartition::cache_neigh_communities(Id v, igraph_neimode_t mode) const noexcept
{
  // TODO: We can probably calculate at once the IN, OUT and ALL
  // rather than this being called multiple times.

  // Weight between vertex and community
  #ifdef DEBUG
    cerr << "Weight MutableVertexPartition::cache_neigh_communities(" << v << ", " << mode << ")." << endl;
  #endif
  vector<Weight>* _cached_weight_tofrom_community = NULL;
  vector<Id>* _cached_neighs = NULL;
  switch (mode)
  {
    case IGRAPH_IN:
      _cached_weight_tofrom_community = &(this->_cached_weight_from_community);
      _cached_neighs = &(this->_cached_neigh_comms_from);
      break;
    case IGRAPH_OUT:
      _cached_weight_tofrom_community = &(this->_cached_weight_to_community);
      _cached_neighs = &(this->_cached_neigh_comms_to);
      break;
    case IGRAPH_ALL:
      _cached_weight_tofrom_community = &(this->_cached_weight_all_community);
      _cached_neighs = &(this->_cached_neigh_comms_all);
      break;
  }

  // Reset cached communities
  std::fill(_cached_weight_tofrom_community->begin(), _cached_weight_tofrom_community->end(), 0);

  // Loop over all incident edges
  vector<Id> const& neighbours = this->graph->get_neighbours(v, mode);
  vector<Id> const& neighbour_edges = this->graph->get_neighbour_edges(v, mode);

  Id degree = neighbours.size();

  // Reset cached neighbours
  _cached_neighs->clear();
  _cached_neighs->reserve(degree);
  for (Id idx = 0; idx < degree; idx++)
  {
    Id u = neighbours[idx];
    Id e = neighbour_edges[idx];

    // If it is an edge to the requested community
    #ifdef DEBUG
      Id u_comm = this->_membership[u];
    #endif
    Id comm = this->_membership[u];
    // Get the weight of the edge
    Weight w = this->graph->edge_weight(e);
    // Self loops appear twice here if the graph is undirected, so divide by 2.0 in that case.
    if (u == v && !this->graph->is_directed())
        w /= 2.0;
    #ifdef DEBUG
      cerr << "\t" << "Edge (" << v << "-" << u << "), Comm (" << comm << "-" << u_comm << ") weight: " << w << "." << endl;
    #endif
    (*_cached_weight_tofrom_community)[comm] += w;
    // REMARK: Notice in the rare case of negative weights, being exactly equal
    // for a certain community, that this community may then potentially be added multiple
    // times to the _cached_neighs. However, I don' believe this causes any further issue,
    // so that's why I leave this here as is.
    if ((*_cached_weight_tofrom_community)[comm] != 0)
      _cached_neighs->push_back(comm);
  }
  #ifdef DEBUG
    cerr << "exit Graph::cache_neigh_communities(" << v << ", " << mode << ")." << endl;
  #endif
}

vector<Id> const& MutableVertexPartition::get_neigh_comms(Id v, igraph_neimode_t mode) const
{
  switch (mode)
  {
    case IGRAPH_IN:
      if (this->_current_node_cache_community_from != v)
      {
        cache_neigh_communities(v, mode);
        this->_current_node_cache_community_from = v;
      }
      return this->_cached_neigh_comms_from;
    case IGRAPH_OUT:
      if (this->_current_node_cache_community_to != v)
      {
        cache_neigh_communities(v, mode);
        this->_current_node_cache_community_to = v;
      }
      return this->_cached_neigh_comms_to;
    case IGRAPH_ALL:
      if (this->_current_node_cache_community_all != v)
      {
        cache_neigh_communities(v, mode);
        this->_current_node_cache_community_all = v;
      }
      return this->_cached_neigh_comms_all;
  }
  throw LeidenException("Problem obtaining neighbour communities, invalid mode.");
}

set<Id> MutableVertexPartition::get_neigh_comms(Id v, igraph_neimode_t mode, vector<Id> const& constrained_membership) const
{
  Id degree = this->graph->degree(v, mode);
  vector<Id> const& neigh = this->graph->get_neighbours(v, mode);
  set<Id> neigh_comms;
  for (Id i=0; i < degree; i++)
  {
    Id u = neigh[i];
    if (constrained_membership[v] == constrained_membership[u])
      neigh_comms.insert( this->membership(u) );
  }
  return neigh_comms;
}
