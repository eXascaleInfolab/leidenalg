#include "LinearResolutionParameterVertexPartition.h"

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(const Graph* graph
	, vector<Id> membership, Weight resolution_parameter): ResolutionParameterVertexPartition(graph
		, membership, resolution_parameter)
{}

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(const Graph* graph
	, vector<Id> membership): ResolutionParameterVertexPartition(graph, membership)
{}

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(const Graph* graph,
  Weight resolution_parameter): ResolutionParameterVertexPartition(graph, resolution_parameter)
{}

LinearResolutionParameterVertexPartition::LinearResolutionParameterVertexPartition(const Graph* graph)
	: ResolutionParameterVertexPartition(graph)
{}

LinearResolutionParameterVertexPartition::~LinearResolutionParameterVertexPartition()
{}
