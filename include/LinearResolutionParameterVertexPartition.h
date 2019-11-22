#ifndef LINEARRESOLUTIONPARAMETERVERTEXPARTITION_H
#define LINEARRESOLUTIONPARAMETERVERTEXPARTITION_H

#include <ResolutionParameterVertexPartition.h>


class LinearResolutionParameterVertexPartition : public ResolutionParameterVertexPartition
{
  public:
    LinearResolutionParameterVertexPartition(const Graph* graph, vector<Id> membership
		, Weight resolution_parameter);
    LinearResolutionParameterVertexPartition(const Graph* graph, vector<Id> membership);
    LinearResolutionParameterVertexPartition(const Graph* graph, Weight resolution_parameter);
    LinearResolutionParameterVertexPartition(const Graph* graph);
    virtual ~LinearResolutionParameterVertexPartition();
};

#endif // RESOLUTIONPARAMETERVERTEXPARTITION_H
