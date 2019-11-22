#ifndef CPMVERTEXPARTITION_H
#define CPMVERTEXPARTITION_H

#include <LinearResolutionParameterVertexPartition.h>


class CPMVertexPartition : public LinearResolutionParameterVertexPartition
{
  public:
    CPMVertexPartition(Graph* graph,
          vector<Id> membership, Weight resolution_parameter);
    CPMVertexPartition(Graph* graph,
          vector<Id> membership);
    CPMVertexPartition(Graph* graph,
      Weight resolution_parameter);
    CPMVertexPartition(Graph* graph);
    virtual ~CPMVertexPartition();
    virtual CPMVertexPartition* create(Graph* graph);
    virtual CPMVertexPartition* create(Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality(Weight resolution_parameter) const;
};

#endif // CPMVERTEXPARTITION_H
