#ifndef RESOLUTIONPARAMETERVERTEXPARTITION_H
#define RESOLUTIONPARAMETERVERTEXPARTITION_H

#include <MutableVertexPartition.h>


class ResolutionParameterVertexPartition : public MutableVertexPartition
{
  public:
    ResolutionParameterVertexPartition(const Graph* graph
      , vector<Id> membership, Weight resolution_parameter);
    ResolutionParameterVertexPartition(const Graph* graph, vector<Id> membership);
    ResolutionParameterVertexPartition(const Graph* graph, Weight resolution_parameter);
    ResolutionParameterVertexPartition(const Graph* graph);
    virtual ~ResolutionParameterVertexPartition();

    Weight resolution_parameter;

    virtual Weight quality() const
    {
      return this->quality(resolution_parameter);
    };

    virtual Weight quality(Weight resolution_parameter) const
    {
      throw LeidenException("Function not implemented. This should be implemented in a derived class, since the base class does not implement a specific method.");
    };
};

#endif // RESOLUTIONPARAMETERVERTEXPARTITION_H
