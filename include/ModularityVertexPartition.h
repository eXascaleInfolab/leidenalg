#ifndef MODULARITYVERTEXPARTITION_H
#define MODULARITYVERTEXPARTITION_H

#include <MutableVertexPartition.h>


class ModularityVertexPartition: public MutableVertexPartition
{
  public:
    ModularityVertexPartition(Graph* graph, vector<Id> const& membership);
    ModularityVertexPartition(Graph* graph);
    virtual ~ModularityVertexPartition();
    virtual ModularityVertexPartition* create(Graph* graph);
    virtual ModularityVertexPartition* create(Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality() const;
};

#endif // MODULARITYVERTEXPARTITION_H
