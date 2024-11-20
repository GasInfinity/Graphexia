#include <Graphexia/Algo/BFS.hpp>
#include <unordered_set>
#include <vector>

namespace gpx {
    BFSState SetupBFS(const gpx::Graph&, usize from, std::optional<usize> to) {
        return {
            to,
            std::vector<usize>{from},
            std::vector<usize>(),
            0,
            std::unordered_set<usize>{from},
            std::vector<usize>() 
        };
    }

    bool IterateBFS(const gpx::Graph& graph, BFSState& state) {
        const std::vector<gpx::Edge>& edges = graph.Edges();

        while (true) {
            if(state.current < state.visiting.size()) {
                usize id = state.visiting[state.current++];
                const std::vector<usize> edgesForVertex = graph.EdgesForVertex(id);

                for (usize adjacentEdge : edgesForVertex) {
                    const Edge& edge = edges[adjacentEdge];
                    usize adjacentVertex = edge.toId;
                    
                    if(adjacentVertex == id) {
                        if(graph.IsDirected()) { // Cannot use this edge
                            continue;
                        } 
                        
                        adjacentVertex = edge.fromId;
                    }

                    std::pair<std::unordered_set<usize>::iterator, bool> in = state.visitedVertices.insert(adjacentVertex);
                    
                    if(!in.second) {
                        continue;
                    }

                    state.toVisit.push_back(adjacentVertex);
                    state.result.push_back(adjacentEdge);
                    
                    if(state.targetVertex && *state.targetVertex == adjacentVertex) { // We finished!
                        state.toVisit.clear();
                        state.visiting.clear();
                        state.current = 0;
                        return true;
                    }
                }

                return false;
            }
            
            if(state.toVisit.empty()) {
                return true;
            }

            state.current = 0;
            state.visiting.clear();
            state.visiting.insert(state.visiting.begin(), state.toVisit.cbegin(), state.toVisit.cend());
            state.toVisit.clear();
        }

        return state.visiting.empty();
    }
}
