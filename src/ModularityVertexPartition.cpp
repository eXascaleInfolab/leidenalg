#include "ModularityVertexPartition.h"

#ifdef DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

ModularityVertexPartition::ModularityVertexPartition(const Graph* graph, vector<Id> const& membership)
  : MutableVertexPartition(graph, membership)
{}

ModularityVertexPartition::ModularityVertexPartition(const Graph* graph): MutableVertexPartition(graph)
{}

ModularityVertexPartition::~ModularityVertexPartition()
{}

ModularityVertexPartition* ModularityVertexPartition::create(const Graph* graph) const
{
  return new ModularityVertexPartition(graph);
}

ModularityVertexPartition* ModularityVertexPartition::create(const Graph* graph, vector<Id> const& membership) const
{
  return new ModularityVertexPartition(graph, membership);
}

/*****************************************************************************
  Returns the difference in modularity if we move a node to a new community
*****************************************************************************/
Weight ModularityVertexPartition::diff_move(Id v, Id new_comm)
{
  #ifdef DEBUG
    cerr << "Weight ModularityVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
  #endif
  Id old_comm = this->_membership[v];
  Weight diff = 0.0;
  Weight total_weight = this->graph->total_weight()*(2.0 - this->graph->is_directed());
  if (total_weight == 0.0)
    return 0.0;
  if (new_comm != old_comm)
  {
    #ifdef DEBUG
      cerr << "\t" << "old_comm: " << old_comm << endl;
    #endif
    Weight w_to_old = this->weight_to_comm(v, old_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_to_old: " << w_to_old << endl;
    #endif
    Weight w_from_old = this->weight_from_comm(v, old_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_from_old: " << w_from_old << endl;
    #endif
    Weight w_to_new = this->weight_to_comm(v, new_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_to_new: " << w_to_new << endl;
    #endif
    Weight w_from_new = this->weight_from_comm(v, new_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_from_new: " << w_from_new << endl;
    #endif
    Weight k_out = this->graph->strength(v, IGRAPH_OUT);
    #ifdef DEBUG
      cerr << "\t" << "k_out: " << k_out << endl;
    #endif
    Weight k_in = this->graph->strength(v, IGRAPH_IN);
    #ifdef DEBUG
      cerr << "\t" << "k_in: " << k_in << endl;
    #endif
    Weight self_weight = this->graph->node_self_weight(v);
    #ifdef DEBUG
      cerr << "\t" << "self_weight: " << self_weight << endl;
    #endif
    Weight K_out_old = this->total_weight_from_comm(old_comm);
    #ifdef DEBUG
      cerr << "\t" << "K_out_old: " << K_out_old << endl;
    #endif
    Weight K_in_old = this->total_weight_to_comm(old_comm);
    #ifdef DEBUG
      cerr << "\t" << "K_in_old: " << K_in_old << endl;
    #endif
    Weight K_out_new = this->total_weight_from_comm(new_comm) + k_out;
    #ifdef DEBUG
      cerr << "\t" << "K_out_new: " << K_out_new << endl;
    #endif
    Weight K_in_new = this->total_weight_to_comm(new_comm) + k_in;
    #ifdef DEBUG
      cerr << "\t" << "K_in_new: " << K_in_new << endl;
      cerr << "\t" << "total_weight: " << total_weight << endl;
    #endif
    Weight diff_old = (w_to_old - k_out*K_in_old/total_weight) + \
               (w_from_old - k_in*K_out_old/total_weight);
    #ifdef DEBUG
      cerr << "\t" << "diff_old: " << diff_old << endl;
    #endif
    Weight diff_new = (w_to_new + self_weight - k_out*K_in_new/total_weight) + \
               (w_from_new + self_weight - k_in*K_out_new/total_weight);
    #ifdef DEBUG
      cerr << "\t" << "diff_new: " << diff_new << endl;
    #endif
    diff = diff_new - diff_old;
    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << endl;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit Weight ModularityVertexPartition::diff_move((" << v << ", " << new_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  Weight m;
  if (this->graph->is_directed())
    m = this->graph->total_weight();
  else
    m = 2*this->graph->total_weight();
  return diff/m;
}


/*****************************************************************************
  Give the modularity of the partition.

  We here use the unscaled version of modularity, in other words, we don"t
  normalise by the number of edges.
******************************************************************************/
Weight ModularityVertexPartition::quality() const
{
  #ifdef DEBUG
    cerr << "Weight ModularityVertexPartition::quality()" << endl;
  #endif
  Weight mod = 0.0;

  Weight m;
  if (this->graph->is_directed())
    m = this->graph->total_weight();
  else
    m = 2*this->graph->total_weight();

  if (m == 0)
    return 0.0;

  for (Id c = 0; c < this->n_communities(); c++)
  {
    Weight w = this->total_weight_in_comm(c);
    Weight w_out = this->total_weight_from_comm(c);
    Weight w_in = this->total_weight_to_comm(c);
    #ifdef DEBUG
      Id csize = this->csize(c);
      cerr << "\t" << "Comm: " << c << ", size=" << csize << ", w=" << w << ", w_out=" << w_out << ", w_in=" << w_in << "." << endl;
    #endif
    mod += w - w_out*w_in/((this->graph->is_directed() ? 1.0 : 4.0)*this->graph->total_weight());
  }
  Weight q = (2.0 - this->graph->is_directed())*mod;
  #ifdef DEBUG
    cerr << "exit Weight ModularityVertexPartition::quality()" << endl;
    cerr << "return " << q/m << endl << endl;
  #endif
  return q/m;
}
