#ifndef _GRAPHEXIA_ALGO_BFS__HPP_
#define _GRAPHEXIA_ALGO_BFS__HPP_

#include <Graphexia/Graph.hpp>

#include <unordered_set>
#include <vector>

namespace gpx {
    struct BFSState {
        std::optional<usize> targetVertex;
        std::vector<usize> visiting;
        std::vector<usize> toVisit;
        usize current;

        std::unordered_set<usize> visitedVertices;
        std::vector<usize> result;
    };

    BFSState SetupBFS(const gpx::Graph& graph, usize from, std::optional<usize> to);
    bool IterateBFS(const gpx::Graph& graph, BFSState& state);
} // namespace gpx

#endif
