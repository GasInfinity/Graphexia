#ifndef _GRAPHEXIA_ALGO_DFS__HPP_
#define _GRAPHEXIA_ALGO_DFS__HPP_

#include <Graphexia/Graph.hpp>

#include <unordered_set>
#include <vector>

namespace gpx {
    struct DFSState {
        std::optional<usize> targetVertex;
        // (Adjacent Vertex, Adjacent Edge)
        std::vector<std::pair<usize, usize>> visiting;
        usize last;

        std::unordered_set<usize> visitedVertices;
        std::vector<usize> result;
    };

    DFSState SetupDFS(const gpx::Graph& graph, usize from, std::optional<usize> to);
    bool IterateDFS(const gpx::Graph& graph, DFSState& state);
} // namespace gpx

#endif
