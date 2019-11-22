#ifndef RBERVERTEXPARTITION_H
#define RBERVERTEXPARTITION_H

#include <LinearResolutionParameterVertexPartition.h>


class RBERVertexPartition : public LinearResolutionParameterVertexPartition
{
  public:
    RBERVertexPartition(Graph* graph, vector<Id> const& membership, Weight resolution_parameter);
    RBERVertexPartition(Graph* graph, vector<Id> const& membership);
    RBERVertexPartition(Graph* graph, Weight resolution_parameter);
    RBERVertexPartition(Graph* graph);
    virtual ~RBERVertexPartition();
    virtual RBERVertexPartition* create(Graph* graph);
    virtual RBERVertexPartition* create(Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality(Weight resolution_parameter) const;
};

#endif // RBERVERTEXPARTITION_H
