#include "SignificanceVertexPartition.h"

#ifdef DEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

SignificanceVertexPartition::SignificanceVertexPartition(Graph* graph
  , vector<Id> const& membership): MutableVertexPartition(graph, membership)
{}

SignificanceVertexPartition::SignificanceVertexPartition(Graph* graph)
  : MutableVertexPartition(graph)
{}

SignificanceVertexPartition* SignificanceVertexPartition::create(Graph* graph)
{
  return new SignificanceVertexPartition(graph);
}

SignificanceVertexPartition* SignificanceVertexPartition::create(Graph* graph, vector<Id> const& membership)
{
  return new SignificanceVertexPartition(graph, membership);
}

SignificanceVertexPartition::~SignificanceVertexPartition()
{}

Weight SignificanceVertexPartition::diff_move(Id v, Id new_comm)
{
  #ifdef DEBUG
    cerr << "virtual Weight SignificanceVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
  #endif
  Id old_comm = this->membership(v);
  Id nsize = this->graph->node_size(v);
  Weight diff = 0.0;
  if (new_comm != old_comm)
  {
    Weight normalise = (2.0 - this->graph->is_directed());
    Weight p = this->graph->density();
    #ifdef DEBUG
      Id n = this->graph->total_size();
      cerr << "\t" << "Community: " << old_comm << " => " << new_comm << "." << endl;
      cerr << "\t" << "n: " << n << ", m: " << this->graph->total_weight() << ", p: " << p << "." << endl;
    #endif

    //Old comm
    Id n_old = this->csize(old_comm);
    Id N_old = this->graph->possible_edges(n_old);
    Weight m_old = this->total_weight_in_comm(old_comm);
    Weight q_old = 0.0;
    if (N_old > 0)
      q_old = m_old/N_old;
    #ifdef DEBUG
      cerr << "\t" << "n_old: " << n_old << ", N_old: " << N_old << ", m_old: " << m_old << ", q_old: " << q_old
           << ", KL: " << KL(q_old, p)  << "." << endl;
    #endif
    // Old comm after move
    Id n_oldx = n_old - nsize; // It should not be possible that this becomes negative, so no need for ptrdiff_t here.
    Id N_oldx = this->graph->possible_edges(n_oldx);
    Weight sw = this->graph->node_self_weight(v);
    // Be careful to exclude the self weight here, because this is include in the weight_to_comm function.
    Weight wtc = this->weight_to_comm(v, old_comm) - sw;
    Weight wfc = this->weight_from_comm(v, old_comm) - sw;
    #ifdef DEBUG
      cerr << "\t" << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    Weight m_oldx = m_old - wtc/normalise - wfc/normalise - sw;
    Weight q_oldx = 0.0;
    if (N_oldx > 0)
      q_oldx = m_oldx/N_oldx;
    #ifdef DEBUG
      cerr << "\t" << "n_oldx: " << n_oldx << ", N_oldx: " << N_oldx << ", m_oldx: " << m_oldx << ", q_oldx: " << q_oldx
           << ", KL: " << KL(q_oldx, p)  << "." << endl;
    #endif

    // New comm
    Id n_new = this->csize(new_comm);
    Id N_new = this->graph->possible_edges(n_new);
    Weight m_new = this->total_weight_in_comm(new_comm);
    Weight q_new = 0.0;
    if (N_new > 0)
      q_new = m_new/N_new;
    #ifdef DEBUG
      cerr << "\t" << "n_new: " << n_new << ", N_new: " << N_new << ", m_new: " << m_new << ", q_new: " << q_new
           << ", KL: " << KL(q_new, p)  << "." << endl;
    #endif

    // New comm after move
    Id n_newx = n_new + nsize;
    Id N_newx = this->graph->possible_edges(n_newx);
    wtc = this->weight_to_comm(v, new_comm);
    wfc = this->weight_from_comm(v, new_comm);
    sw = this->graph->node_self_weight(v);
    #ifdef DEBUG
      cerr << "\t" << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    Weight m_newx = m_new + wtc/normalise + wfc/normalise + sw;
    Weight q_newx = 0.0;
    if (N_newx > 0)
      q_newx = m_newx/N_newx;
    #ifdef DEBUG
      cerr << "\t" << "n_newx: " << n_newx << ", N_newx: " << N_newx << ", m_newx: " << m_newx
           << ", q_newx: " << q_newx
           << ", KL: " << KL(q_newx, p) << "." << endl;
    #endif

    // Calculate actual diff

    diff =   (Weight)N_oldx*KLL(q_oldx, p) + (Weight)N_newx*KLL(q_newx, p)
           - (Weight)N_old *KLL(q_old,  p) - (Weight)N_new *KLL(q_new,  p);
    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << "." << endl;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit Weight SignificanceVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  return diff;
}

/********************************************************************************
   Calculate the significance of the partition.
*********************************************************************************/
Weight SignificanceVertexPartition::quality() const
{
  #ifdef DEBUG
    cerr << "Weight SignificanceVertexPartition::quality()";
    Id n = this->graph->total_size();
  #endif
  Weight S = 0.0;
  Weight p = this->graph->density();
  #ifdef DEBUG
    cerr << "\t" << "n=" << n << ", m=" << this->graph->total_weight() << ", p=" << p << "." << endl;
  #endif
  for (Id c = 0; c < this->n_communities(); c++)
  {
    Id n_c = this->csize(c);
    Weight m_c = this->total_weight_in_comm(c);
    Weight p_c = 0.0;
    Id N_c = this->graph->possible_edges(n_c);
    if (N_c > 0)
      p_c = m_c/N_c;
    #ifdef DEBUG
      cerr << "\t" << "c=" << c << ", n_c=" << n_c << ", m_c=" << m_c << ", N_c=" << N_c
         << ", p_c=" << p_c << ", p=" << p << ", KLL=" << KL(p_c, p) << "." << endl;
    #endif
    S += N_c*KLL(p_c, p);
  }
  #ifdef DEBUG
    cerr << "exit SignificanceVertexPartition::quality()" << endl;
    cerr << "return " << S << endl << endl;
  #endif
  return S;
}
