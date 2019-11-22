#include "SurpriseVertexPartition.h"
using std::cerr;
using std::endl;


SurpriseVertexPartition::SurpriseVertexPartition(const Graph* graph, vector<Id> const& membership)
  : MutableVertexPartition(graph, membership)
{}

SurpriseVertexPartition::SurpriseVertexPartition(const Graph* graph): MutableVertexPartition(graph)
{}

SurpriseVertexPartition* SurpriseVertexPartition::create(const Graph* graph)
{
  return new SurpriseVertexPartition(graph);
}

 SurpriseVertexPartition*  SurpriseVertexPartition::create(const Graph* graph, vector<Id> const& membership)
{
  return new  SurpriseVertexPartition(graph, membership);
}

SurpriseVertexPartition::~SurpriseVertexPartition()
{}

Weight SurpriseVertexPartition::diff_move(Id v, Id new_comm)
{
  #ifdef DEBUG
    cerr << "virtual Weight SurpriseVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
  #endif
  Id old_comm = this->membership(v);
  Id nsize = this->graph->node_size(v);
  #ifdef DEBUG
    cerr << "\t" << "nsize: " << nsize << endl;
  #endif
  Weight diff = 0.0;
  Weight m = this->graph->total_weight();

  if(!m)  // Note: strict comparison is fine here
    return 0;

  if (new_comm != old_comm)
  {
    Weight normalise = (2.0 - this->graph->is_directed());
    Id n = this->graph->total_size();
    Id n2 = this->graph->possible_edges(n);

    #ifdef DEBUG
      cerr << "\t" << "Community: " << old_comm << " => " << new_comm << "." << endl;
      cerr << "\t" << "m: " << m << ", n2: " << n2 << "." << endl;
    #endif

    // Before move
    Weight mc = this->total_weight_in_all_comms();
    Id nc2 = this->total_possible_edges_in_all_comms();
    #ifdef DEBUG
      cerr << "\t" << "mc: " << mc << ", nc2: " << nc2 << "." << endl;
    #endif

    // To old comm
    Id n_old = this->csize(old_comm);
    Weight sw = this->graph->node_self_weight(v);
    Weight wtc = this->weight_to_comm(v, old_comm) - sw;
    Weight wfc = this->weight_from_comm(v, old_comm) - sw;
    #ifdef DEBUG
      cerr << "\t"  << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    Weight m_old = wtc/normalise + wfc/normalise + sw;
    #ifdef DEBUG
      cerr << "\t" << "m_old: " << m_old << ", n_old: " << n_old << "." << endl;
    #endif

    // To new comm
    Id n_new = this->csize(new_comm);
    wtc = this->weight_to_comm(v, new_comm);
    wfc = this->weight_from_comm(v, new_comm);
    sw = this->graph->node_self_weight(v);
    #ifdef DEBUG
      cerr << "\t"  << "wtc: " << wtc << ", wfc: " << wfc << ", sw: " << sw << "." << endl;
    #endif
    Weight m_new = wtc/normalise + wfc/normalise + sw;
    #ifdef DEBUG
      cerr << "\t" << "m_new: " << m_new << ", n_new: " << n_new << "." << endl;
    #endif

    Weight q = mc/m;
    Weight s = (Weight)nc2/(Weight)n2;
    Weight q_new = (mc - m_old + m_new)/m;
    #ifdef DEBUG
      cerr << "\t" << "mc - m_old + m_new=" << (mc - m_old + m_new) << endl;
    #endif
    Weight delta_nc2 = 2.0*nsize*(ptrdiff_t)(n_new - n_old + nsize)/normalise;
    Weight s_new = (Weight)(nc2 + delta_nc2)/(Weight)n2;
    #ifdef DEBUG
      cerr << "\t" << "delta_nc2=" << delta_nc2 << endl;
    #endif
    #ifdef DEBUG
      cerr << "\t" << "q:\t" << q << ", s:\t"  << s << "." << endl;
      cerr << "\t" << "q_new:\t" << q_new << ", s_new:\t"  << s_new << "." << endl;
    #endif
    diff = m*(KLL(q_new, s_new) - KLL(q, s));

    #ifdef DEBUG
      cerr << "\t" << "diff: " << diff << "." << endl;
    #endif
  }
  #ifdef DEBUG
    cerr << "exit Weight SurpriseVertexPartition::diff_move(" << v << ", " << new_comm << ")" << endl;
    cerr << "return " << diff << endl << endl;
  #endif
  return diff;
}

Weight SurpriseVertexPartition::quality() const
{
  #ifdef DEBUG
    cerr << "Weight SurpriseVertexPartition::quality()" << endl;
  #endif

  Weight mc = this->total_weight_in_all_comms();
  Id nc2 = this->total_possible_edges_in_all_comms();
  Weight m = this->graph->total_weight();
  Id n = this->graph->total_size();

  if(!m)  // Note: strict comparison is fine here
    return 0;

  Id n2 = this->graph->possible_edges(n);

  #ifdef DEBUG
    cerr << "\t" << "mc=" << mc << ", m=" << m << ", nc2=" << nc2 << ", n2=" << n2 << "." << endl;
  #endif
  Weight q = mc/m;
  Weight s = (Weight)nc2/(Weight)n2;
  #ifdef DEBUG
    cerr << "\t" << "q:\t" << q << ", s:\t"  << s << "." << endl;
  #endif
  Weight S = m*KLL(q,s);
  #ifdef DEBUG
    cerr << "exit SurpriseVertexPartition::quality()" << endl;
    cerr << "return " << S << endl << endl;
  #endif
  return S;
}
