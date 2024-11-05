#include <Graphexia/graph.hpp>

namespace gpx {
    usize Graph::FindVertex(i16 x, i16 y) const {
        for (usize i = 0; i < this->vertices.size(); ++i) {
            if(this->vertices[i].collides(x, y)) {
                return i;
            }
        }

        return NoVertex;
    }

    std::vector<usize> Graph::GetAdjacencyMatrix() const {
        usize verticesCount = this->vertices.size();
        usize matrixLength = verticesCount * verticesCount;
        std::vector<usize> adjacency(matrixLength);

        for (usize i = 0; i < verticesCount; ++i) {
            std::vector<usize> edgeIds = this->edgesForVertex[i];

            usize currentRow = i * verticesCount;
            for (usize j = 0; j < edgeIds.size(); ++j) {
                usize edgeId = edgeIds[j];
                const Edge& currentEdge = this->edges[edgeId];
                
                usize rowId = currentEdge.toId;
                if(i != currentEdge.fromId) {
                    if(this->IsDirected()) {
                        continue;
                    }

                    rowId = currentEdge.fromId;
                }

                ++adjacency[currentRow + rowId];
            } 
        }

        return adjacency;
    }

    std::vector<IncidenceState> Graph::GetIncidenceMatrix() const {
        usize verticesCount = this->vertices.size();
        usize edgesCount = this->edges.size();
        usize matrixLength = verticesCount * edgesCount;
        std::vector<IncidenceState> incidence(matrixLength);

        for (usize i = 0; i < verticesCount; ++i) {
            std::vector<usize> edgeIds = this->edgesForVertex[i];

            usize currentRow = i * edgesCount;
            for (usize j = 0; j < edgeIds.size(); ++j) {
                usize edgeId = edgeIds[j];
                const Edge& edge = this->edges[edgeId];

                IncidenceState state = this->IsDirected() && edge.fromId == i ? IncidenceState::Leaves : static_cast<IncidenceState>(static_cast<i8>(IncidenceState::Incident) + (edge.fromId == edge.toId));
                incidence[currentRow + edgeId] = state;
            }
        }

        return incidence;
    }
} // namespace gpx
