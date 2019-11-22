#ifndef SURPRISEVERTEXPARTITION_H
#define SURPRISEVERTEXPARTITION_H

#include "MutableVertexPartition.h"
#include <iostream>


class SurpriseVertexPartition : public MutableVertexPartition
{
  public:
    SurpriseVertexPartition(Graph* graph, vector<Id> const& membership);
    SurpriseVertexPartition(Graph* graph, SurpriseVertexPartition* partition);
    SurpriseVertexPartition(Graph* graph);
    virtual ~SurpriseVertexPartition();
    virtual SurpriseVertexPartition* create(Graph* graph);
    virtual SurpriseVertexPartition* create(Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality() const;
};

#endif // SURPRISEVERTEXPARTITION_H
