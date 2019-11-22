#ifndef MUTABLEVERTEXPARTITION_H
#define MUTABLEVERTEXPARTITION_H

#include <string>
#include "GraphHelper.h"
#include <map>
#include <set>
#include <utility>
#include <algorithm>

using std::string;
using std::map;
using std::set;
using std::make_pair;
using std::pair;
using std::sort;
using std::reverse;


/****************************************************************************
Contains a partition of graph.

This class contains the basic implementation for optimizing a partition.
Specifically, it implements all the administration necessary to keep track of
the partition from various points of view. Internally, it keeps track of the
number of internal edges (or total weight), the size of the communities, the
total incoming degree (or weight) for a community, etc... When deriving from
this class, one can easily use this administration to provide their own
implementation.

In order to keep the administration up-to-date, all changes in partition
should be done through move_node. This function moves a node from one
community to another, and updates all the administration.

It is possible to manually update the membership vector, and then call
__init_admin() which completely refreshes all the administration. This is
only possible by updating the membership vector, not by changing some of the
other variables.

The basic idea is that diff_move computes the difference in the quality
function if we call move_node for the same move. Using this framework, the
Leiden method in the optimisation class can call these general functions in
order to optimise the quality function.
*****************************************************************************/
class MutableVertexPartition
{
  public:
    MutableVertexPartition(const Graph* graph, vector<Id> const& membership);
    MutableVertexPartition(const Graph* graph);
//    //! \brief Acquire the graph and delete it on destruction
//    //!
//    //! \param graph Graph*&  - graph to be acquired
//    //! \param membership vector<Id> const*  - possible membership vector to be copied
//    MutableVertexPartition(Graph*& graph,
//        vector<Id> const* membership=nullptr);
    MutableVertexPartition(const MutableVertexPartition& other)=delete;
    MutableVertexPartition(MutableVertexPartition&& other) noexcept;

    virtual ~MutableVertexPartition();

    virtual MutableVertexPartition& operator=(const MutableVertexPartition&)=delete;
    virtual MutableVertexPartition& operator=(MutableVertexPartition&& other) noexcept;

    virtual MutableVertexPartition* create(const Graph* graph) const;
    virtual MutableVertexPartition* create(const Graph* graph, vector<Id> const& membership) const;

    inline Id membership(Id v) const noexcept { return this->_membership[v]; };
    inline vector<Id> const& membership() const noexcept { return this->_membership; };

    Id csize(Id comm) const noexcept;
    Id cnodes(Id comm) const noexcept;
    vector<Id> get_community(Id comm) const noexcept;
    vector< vector<Id> > get_communities() const noexcept;
    Id n_communities() const noexcept;

    void move_node(Id v,Id new_comm);
    virtual Weight diff_move(Id v, Id new_comm)
    {
      throw LeidenException("Function not implemented. This should be implemented in a derived class, since the base class does not implement a specific method.");
    };
    virtual Weight quality() const
    {
      throw LeidenException("Function not implemented. This should be implemented in a derived class, since the base class does not implement a specific method.");
    };

    inline const Graph* get_graph() const noexcept  { return this->graph; };

    void renumber_communities();
    void renumber_communities(vector<Id> const& new_membership);
    void set_membership(vector<Id> const& new_membership);
    vector<Id> static renumber_communities(vector<MutableVertexPartition*> partitions);
    Id get_empty_community();
    Id add_empty_community();
    void from_coarse_partition(vector<Id> const& coarse_partition_membership);
    void from_coarse_partition(MutableVertexPartition* partition);
    void from_coarse_partition(MutableVertexPartition* partition, vector<Id> const& coarser_membership);
    void from_coarse_partition(vector<Id> const& coarse_partition_membership, vector<Id> const& coarse_node);

    void from_partition(MutableVertexPartition* partition);

    inline Weight total_weight_in_comm(Id comm) const noexcept  { return comm < _n_communities ? this->_total_weight_in_comm[comm] : 0.0; };
    inline Weight total_weight_from_comm(Id comm) const noexcept  { return comm < _n_communities ? this->_total_weight_from_comm[comm] : 0.0; };
    inline Weight total_weight_to_comm(Id comm) const noexcept { return comm < _n_communities ? this->_total_weight_to_comm[comm] : 0.0; };

    inline Weight total_weight_in_all_comms() const noexcept  { return _total_weight_in_all_comms; };
    inline Id total_possible_edges_in_all_comms() const noexcept  { return _total_possible_edges_in_all_comms; };

    Weight weight_to_comm(Id v, Id comm) const noexcept;
    Weight weight_from_comm(Id v, Id comm) const noexcept;

    vector<Id> const& get_neigh_comms(Id v, igraph_neimode_t) const;
    set<Id> get_neigh_comms(Id v, igraph_neimode_t mode, vector<Id> const& constrained_membership) const;

  protected:

    void init_admin();

    vector<Id> _membership; // Membership vector, i.e. \sigma_i = c means that node i is in community c

    // Note: the graph is automatically deleted on destruction if its owner is this partition
    const Graph* graph;

    // Community size
    vector< Id > _csize;

    // Number of nodes in community
    vector< Id > _cnodes;

    Weight weight_vertex_tofrom_comm(Id v, Id comm, igraph_neimode_t mode);

    void set_default_attrs();

  private:

    // Keep track of the internal weight of each community
    vector<Weight> _total_weight_in_comm;
    // Keep track of the total weight to a community
    vector<Weight> _total_weight_to_comm;
    // Keep track of the total weight from a community
    vector<Weight> _total_weight_from_comm;
    // Keep track of the total internal weight
    Weight _total_weight_in_all_comms;
    Id _total_possible_edges_in_all_comms;
    Id _n_communities;

    vector<Id> _empty_communities;

    void cache_neigh_communities(Id v, igraph_neimode_t mode) const noexcept;

    mutable Id _current_node_cache_community_from; mutable vector<Weight> _cached_weight_from_community; mutable vector<Id> _cached_neigh_comms_from;
    mutable Id _current_node_cache_community_to;   mutable vector<Weight> _cached_weight_to_community;   mutable vector<Id> _cached_neigh_comms_to;
    mutable Id _current_node_cache_community_all;  mutable vector<Weight> _cached_weight_all_community;  mutable vector<Id> _cached_neigh_comms_all;

    void clean_mem();
    void init_graph_admin();

    void update_n_communities();

};

#endif // MUTABLEVERTEXPARTITION_H
