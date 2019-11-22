#ifndef RBERVERTEXPARTITION_H
#define RBERVERTEXPARTITION_H

#include <LinearResolutionParameterVertexPartition.h>


class RBERVertexPartition : public LinearResolutionParameterVertexPartition
{
  public:
    RBERVertexPartition(const Graph* graph, vector<Id> const& membership, Weight resolution_parameter);
    RBERVertexPartition(const Graph* graph, vector<Id> const& membership);
    RBERVertexPartition(const Graph* graph, Weight resolution_parameter);
    RBERVertexPartition(const Graph* graph);
    virtual ~RBERVertexPartition();
    virtual RBERVertexPartition* create(const Graph* graph) const;
    virtual RBERVertexPartition* create(const Graph* graph, vector<Id> const& membership) const;

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality(Weight resolution_parameter) const;
};

#endif // RBERVERTEXPARTITION_H
