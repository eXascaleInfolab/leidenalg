#ifndef SIGNIFICANCEVERTEXPARTITION_H
#define SIGNIFICANCEVERTEXPARTITION_H

#include <MutableVertexPartition.h>


class SignificanceVertexPartition : public MutableVertexPartition
{
  public:
    SignificanceVertexPartition(const Graph* graph, vector<Id> const& membership);
    SignificanceVertexPartition(const Graph* graph);
    virtual ~SignificanceVertexPartition();
    virtual SignificanceVertexPartition* create(const Graph* graph);
    virtual SignificanceVertexPartition* create(const Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality() const;
};

#endif // SIGNIFICANCEVERTEXPARTITION_H
