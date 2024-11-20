#include <Graphexia/Algo/Kruskal.hpp>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <vector>

namespace gpx {
    KruskalState SetupKruskal(const gpx::Graph& graph) {
        const std::vector<Edge>& edges = graph.Edges();

        std::vector<usize> sortedEdges(graph.Edges().size());
        std::iota(sortedEdges.begin(), sortedEdges.end(), 0);
        std::sort(sortedEdges.begin(), sortedEdges.end(), [&edges](usize lhs, usize rhs) {
            return edges[lhs].weight < edges[rhs].weight;
        });

        std::vector<std::unordered_set<usize>> indirectConnections; 
        indirectConnections.reserve(edges.size() >> 2);

        // Indires stored as (i + 1), where i == 0 is an invalid index
        std::vector<usize> connections(graph.Vertices());

        std::vector<usize> finalEdges;
        finalEdges.reserve((edges.size() * 3) >> 1);

        return KruskalState{
            std::move(sortedEdges),
            std::move(indirectConnections),
            std::move(connections),
            0,
            std::move(finalEdges)
        };
    }

    bool IterateKruskal(const gpx::Graph &graph, KruskalState& state) {
        const std::vector<Edge>& edges = graph.Edges();

        while (state.current < state.sortedEdges.size()) {
            usize edgeId = state.sortedEdges[state.current];
            const Edge& edge = edges[state.sortedEdges[state.current++]];

            usize fromId = edge.fromId;
            usize toId = edge.toId;

            usize& fromConnectionsId = state.connections[fromId];
            usize& toConnectionsId = state.connections[toId];

            // Only connected to themselves
            if(!fromConnectionsId && !toConnectionsId) {
                usize indirectId = state.indirectConnections.size() + 1;
                fromConnectionsId = indirectId;
                toConnectionsId = indirectId; 
                state.indirectConnections.push_back(std::unordered_set{fromId, toId});
            } else if(!fromConnectionsId) {
                std::unordered_set<usize>& connections = state.indirectConnections[toConnectionsId - 1];
                connections.emplace(fromId);
                fromConnectionsId = toConnectionsId;
            } else if(!toConnectionsId) {
                std::unordered_set<usize>& connections = state.indirectConnections[fromConnectionsId - 1];
                connections.emplace(toId);
                toConnectionsId = fromConnectionsId;
            } else { // The two vertices have at least other vertex adjacent to it
                std::unordered_set<usize>& fromConnections = state.indirectConnections[fromConnectionsId - 1];
                std::unordered_set<usize>& toConnections = state.indirectConnections[toConnectionsId - 1];
                
                if(fromConnections.contains(toId)) {
                    continue; // Skip this edge, it will create a loop
                }

                fromConnections.merge(toConnections);
                
                // Fast path, if we connected all the vertices, finish.
                if(fromConnections.size() == graph.Vertices()) {
                    state.result.push_back(edgeId);
                    state.current = state.sortedEdges.size();
                    return true;
                }

                for (const usize id : fromConnections) {
                    state.connections[id] = fromConnectionsId;                
                }
            }

            state.result.push_back(edgeId);
            return state.current == state.sortedEdges.size();
        }

        return true;
    }
}
