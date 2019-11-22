#ifndef RBCONFIGURATIONVERTEXPARTITION_H
#define RBCONFIGURATIONVERTEXPARTITION_H

#include "LinearResolutionParameterVertexPartition.h"


class RBConfigurationVertexPartition : public LinearResolutionParameterVertexPartition
{
  public:
    RBConfigurationVertexPartition(Graph* graph, vector<Id> const& membership
      , Weight resolution_parameter);
    RBConfigurationVertexPartition(Graph* graph, vector<Id> const& membership);
    RBConfigurationVertexPartition(Graph* graph, Weight resolution_parameter);
    RBConfigurationVertexPartition(Graph* graph);
    virtual ~RBConfigurationVertexPartition();
    virtual RBConfigurationVertexPartition* create(Graph* graph);
    virtual RBConfigurationVertexPartition* create(Graph* graph, vector<Id> const& membership);

    virtual Weight diff_move(Id v, Id new_comm);
    virtual Weight quality(Weight resolution_parameter) const;
};

#endif // RBCONFIGURATIONVERTEXPARTITION_H
