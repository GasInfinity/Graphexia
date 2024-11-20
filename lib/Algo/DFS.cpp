#include <Graphexia/Algo/DFS.hpp>

#include <unordered_set>
#include <utility>
#include <vector>

namespace gpx {
    DFSState SetupDFS(const gpx::Graph& graph, usize from, std::optional<usize> to) {
        const std::vector<Edge>& edges = graph.Edges();
        const std::vector<usize>& adjacent = graph.EdgesForVertex(from);

        std::vector<std::pair<usize, usize>> visiting;
        visiting.reserve(adjacent.size());
        
        for (const usize adjacentEdge : adjacent) {
            const Edge& edge = edges[adjacentEdge];
            usize adjacentVertex = edge.toId;
                    
            if(adjacentVertex == from) {
                if(edge.fromId == from || graph.IsDirected()) { // Cannot use this edge
                    continue;
                } 
                
                adjacentVertex = edge.fromId;
            }

            visiting.push_back(std::make_pair(adjacentVertex, adjacentEdge));
        }

        return {
            to,
            std::move(visiting),
            from,

            std::unordered_set<usize>{from},
            std::vector<usize>()
        };
    }

    bool IterateDFS(const Graph& graph, DFSState& state) {
        const std::vector<gpx::Edge>& edges = graph.Edges();

        while(!state.visiting.empty()) {
            auto [vertexId, edgeId] = state.visiting.back();
            state.visiting.pop_back();
            
            // It may happen when graphs have loops!
            if(state.visitedVertices.contains(vertexId)) {
                continue;
            }

            const std::vector<usize>& adjacentEdges = graph.EdgesForVertex(vertexId);
            for(const usize adjacentEdge : adjacentEdges) {
                const Edge& edge = edges[adjacentEdge];
                usize adjacentVertex = edge.toId;
                        
                if(adjacentVertex == vertexId) {
                    if(graph.IsDirected()) { // Cannot use this edge
                        continue;
                    } 
                    
                    adjacentVertex = edge.fromId;
                }
                
                if(state.visitedVertices.contains(adjacentVertex)) { // Already visited
                    continue;
                }
                
                state.visiting.push_back(std::make_pair(adjacentVertex, adjacentEdge));
            } 

            state.last = vertexId;
            state.visitedVertices.emplace(vertexId);
            state.result.push_back(edgeId);
            return false;
        }

        return true;
    }
}
