#include "ResolutionParameterVertexPartition.h"

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(const Graph* graph,
  vector<Id> membership, Weight resolution_parameter)
  : MutableVertexPartition(graph, membership), resolution_parameter(resolution_parameter)
{}

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(const Graph* graph,
  vector<Id> membership): MutableVertexPartition(graph, membership), resolution_parameter(1.0)
{}

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(const Graph* graph,
  Weight resolution_parameter)
  : MutableVertexPartition(graph), resolution_parameter(resolution_parameter)
{}

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(const Graph* graph)
	: MutableVertexPartition(graph), resolution_parameter(1.0)
{}

ResolutionParameterVertexPartition::~ResolutionParameterVertexPartition()
{}
