#include "CPMVertexPartition.h"

CPMVertexPartition::CPMVertexPartition(Graph* graph,
      vector<Id> membership, Weight resolution_parameter) :
        LinearResolutionParameterVertexPartition(graph,
        membership, resolution_parameter)
{ }

CPMVertexPartition::CPMVertexPartition(Graph* graph,
      vector<Id> membership) :
        LinearResolutionParameterVertexPartition(graph,
        membership)
{ }

CPMVertexPartition::CPMVertexPartition(Graph* graph,
      Weight resolution_parameter) :
        LinearResolutionParameterVertexPartition(graph, resolution_parameter)
{ }

CPMVertexPartition::CPMVertexPartition(Graph* graph) :
        LinearResolutionParameterVertexPartition(graph)
{ }

CPMVertexPartition::~CPMVertexPartition()
{ }

CPMVertexPartition* CPMVertexPartition::create(Graph* graph)
{
  return new CPMVertexPartition(graph, this->resolution_parameter);
}

CPMVertexPartition* CPMVertexPartition::create(Graph* graph, vector<Id> const& membership)
{
  return new CPMVertexPartition(graph, membership, this->resolution_parameter);
}

/********************************************************************************
  RBER implementation of a vertex partition
  (which includes a resolution parameter).
 ********************************************************************************/
Weight CPMVertexPartition::diff_move(Id v, Id new_comm)
{
  #ifdef DEBUG
    cerr << "Weight CPMVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
    cerr << "Using resolution parameter: " << this->resolution_parameter << "." << endl;
  #endif
  Id old_comm = this->membership(v);
  Weight diff = 0.0;
  if (new_comm != old_comm)
  {
    Weight w_to_old = this->weight_to_comm(v, old_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_to_old: " << w_to_old << endl;
    #endif
    Weight w_to_new = this->weight_to_comm(v, new_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_to_new: " << w_to_new << endl;
    #endif
    Weight w_from_old = this->weight_from_comm(v, old_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_from_old: " << w_from_old << endl;
    #endif
    Weight w_from_new = this->weight_from_comm(v, new_comm);
    #ifdef DEBUG
      cerr << "\t" << "w_from_new: " << w_from_new << endl;
    #endif
    Id nsize = this->graph->node_size(v);
    #ifdef DEBUG
      cerr << "\t" << "nsize: " << nsize << endl;
    #endif
    Id csize_old = this->csize(old_comm);
    #ifdef DEBUG
      cerr << "\t" << "csize_old: " << csize_old << endl;
    #endif
    Id csize_new = this->csize(new_comm);
    #ifdef DEBUG
      cerr << "\t" << "csize_new: " << csize_new << endl;
    #endif
    Weight self_weight = this->graph->node_self_weight(v);
    #ifdef DEBUG
      cerr << "\t" << "self_weight: " << self_weight << endl;
      cerr << "\t" << "density: " << this->graph->density() << endl;
    #endif
    Weight possible_edge_difference_old = 0.0;
    if (this->graph->correct_self_loops())
      possible_edge_difference_old = nsize*(2.0*csize_old - nsize);
    else
      possible_edge_difference_old = nsize*(2.0*csize_old - nsize - 1.0);
    #ifdef DEBUG
      cerr << "\t" << "possible_edge_difference_old: " << possible_edge_difference_old << endl;
    #endif
    Weight diff_old = w_to_old + w_from_old -
        self_weight - this->resolution_parameter*possible_edge_difference_old;
    #ifdef DEBUG
      cerr << "\t" << "diff_old: " << diff_old << endl;
    #endif
    Weight possible_edge_difference_new = 0.0;
    if (this->graph->correct_self_loops())
      possible_edge_difference_new = nsize*(2.0*csize_new + nsize);
    else
      possible_edge_difference_new = nsize*(2.0*csize_new + nsize - 1.0);
    #ifdef DEBUG
      cerr << "\t" << "possible_edge_difference_new: " << possible_edge_difference_new << endl;
    #endif
    Weight diff_new = w_to_new + w_from_new + self_weight -
        this->resolution_parameter*possible_edge_difference_new;
    #ifdef DEBUG
      cerr << "\t" << "diff_new: " << diff_new << endl;
    #endif
    diff = diff_new - diff_old;
    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << endl;;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit CPMVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  return diff;
}

Weight CPMVertexPartition::quality(Weight resolution_parameter) const
{
  #ifdef DEBUG
    cerr << "Weight CPMVertexPartition::quality()" << endl;
  #endif
  Weight mod = 0.0;
  for (Id c = 0; c < this->n_communities(); c++)
  {
    Id csize = this->csize(c);
    Weight w = this->total_weight_in_comm(c);
    Id comm_possible_edges = this->graph->possible_edges(csize);

    #ifdef DEBUG
      cerr << "\t" << "Comm: " << c << ", w_c=" << w << ", n_c=" << csize << ", comm_possible_edges=" << comm_possible_edges << ", p=" << this->graph->density() << "." << endl;
    #endif
    mod += w - resolution_parameter*comm_possible_edges;
  }
  #ifdef DEBUG
    cerr << "exit Weight CPMVertexPartition::quality()" << endl;
    cerr << "return " << mod << endl << endl;
  #endif
  return (2.0 - this->graph->is_directed())*mod;
}

