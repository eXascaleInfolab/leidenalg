#ifndef SIGNIFICANCEVERTEXPARTITION_H
#define SIGNIFICANCEVERTEXPARTITION_H

#include <MutableVertexPartition.h>


class SignificanceVertexPartition : public MutableVertexPartition
{
  public:
    SignificanceVertexPartition(Graph* graph, vector<Id> const& membership);
    SignificanceVertexPartition(Graph* graph);
    virtual ~SignificanceVertexPartition();
    virtual SignificanceVertexPartition* create(Graph* graph);
    virtual SignificanceVertexPartition* create(Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality() const;
};

#endif // SIGNIFICANCEVERTEXPARTITION_H
