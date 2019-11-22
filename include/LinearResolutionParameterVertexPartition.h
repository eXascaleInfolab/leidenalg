#ifndef LINEARRESOLUTIONPARAMETERVERTEXPARTITION_H
#define LINEARRESOLUTIONPARAMETERVERTEXPARTITION_H

#include <ResolutionParameterVertexPartition.h>


class LinearResolutionParameterVertexPartition : public ResolutionParameterVertexPartition
{
  public:
    LinearResolutionParameterVertexPartition(Graph* graph, vector<Id> membership
		, Weight resolution_parameter);
    LinearResolutionParameterVertexPartition(Graph* graph, vector<Id> membership);
    LinearResolutionParameterVertexPartition(Graph* graph, Weight resolution_parameter);
    LinearResolutionParameterVertexPartition(Graph* graph);
    virtual ~LinearResolutionParameterVertexPartition();
};

#endif // RESOLUTIONPARAMETERVERTEXPARTITION_H
