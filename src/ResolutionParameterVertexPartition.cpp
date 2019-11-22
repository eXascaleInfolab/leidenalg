#include "ResolutionParameterVertexPartition.h"

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(Graph* graph,
  vector<Id> membership, Weight resolution_parameter)
  : MutableVertexPartition(graph, membership)
{ this->resolution_parameter = resolution_parameter; }

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(Graph* graph,
  vector<Id> membership): MutableVertexPartition(graph, membership)
{ this->resolution_parameter = 1.0; }

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(Graph* graph,
	Weight resolution_parameter): MutableVertexPartition(graph)
{ this->resolution_parameter = resolution_parameter;  }

ResolutionParameterVertexPartition::ResolutionParameterVertexPartition(Graph* graph)
	: MutableVertexPartition(graph)
{ this->resolution_parameter = 1.0;  }

ResolutionParameterVertexPartition::~ResolutionParameterVertexPartition()
{}
