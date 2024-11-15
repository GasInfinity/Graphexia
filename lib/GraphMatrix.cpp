#include <Graphexia/GraphMatrix.hpp>

namespace gpx {
    std::vector<usize> AdjacencyMatrix(const Graph& graph) {
        const std::vector<Edge>& graphEdges = graph.Edges();

        usize verticesCount = graph.Vertices();
        usize matrixLength = verticesCount * verticesCount;
        std::vector<usize> adjacency(matrixLength);

        for (usize i = 0; i < verticesCount; ++i) {
            std::vector<usize> edgeIds = graph.EdgesForVertex(i);

            usize currentRow = i * verticesCount;
            for (usize j = 0; j < edgeIds.size(); ++j) {
                usize edgeId = edgeIds[j];
                const Edge& currentEdge = graphEdges[edgeId];
                
                usize rowId = currentEdge.toId;
                if(i != currentEdge.fromId) {
                    if(graph.IsDirected()) {
                        continue;
                    }

                    rowId = currentEdge.fromId;
                }

                ++adjacency[currentRow + rowId];
            } 
        }

        return adjacency;
    }

    std::vector<IncidenceState> IncidenceMatrix(const Graph& graph) {
        const std::vector<Edge>& graphEdges = graph.Edges();

        usize verticesCount = graph.Vertices();
        usize edgesCount = graphEdges.size();
        usize matrixLength = verticesCount * edgesCount;
        std::vector<IncidenceState> incidence(matrixLength);

        for (usize i = 0; i < verticesCount; ++i) {
            std::vector<usize> edgeIds = graph.EdgesForVertex(i);

            usize currentRow = i * edgesCount;
            for (usize j = 0; j < edgeIds.size(); ++j) {
                usize edgeId = edgeIds[j];
                const Edge& edge = graphEdges[edgeId];

                IncidenceState state = graph.IsDirected() && edge.fromId == i ? IncidenceState::Leaves : static_cast<IncidenceState>(static_cast<i8>(IncidenceState::Incident) + (edge.fromId == edge.toId));
                incidence[currentRow + edgeId] = state;
            }
        }

        return incidence;
    }
} // namespace gpx
