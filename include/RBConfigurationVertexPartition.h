#ifndef RBCONFIGURATIONVERTEXPARTITION_H
#define RBCONFIGURATIONVERTEXPARTITION_H

#include "LinearResolutionParameterVertexPartition.h"


class RBConfigurationVertexPartition : public LinearResolutionParameterVertexPartition
{
  public:
    RBConfigurationVertexPartition(const Graph* graph, vector<Id> const& membership
      , Weight resolution_parameter);
    RBConfigurationVertexPartition(const Graph* graph, vector<Id> const& membership);
    RBConfigurationVertexPartition(const Graph* graph, Weight resolution_parameter);
    RBConfigurationVertexPartition(const Graph* graph);
    virtual ~RBConfigurationVertexPartition();
    virtual RBConfigurationVertexPartition* create(const Graph* graph);
    virtual RBConfigurationVertexPartition* create(const Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality(Weight resolution_parameter) const;
};

#endif // RBCONFIGURATIONVERTEXPARTITION_H
