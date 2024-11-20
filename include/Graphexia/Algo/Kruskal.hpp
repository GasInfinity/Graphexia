#ifndef _GRAPHEXIA_ALGO_KRUSKAL__HPP_
#define _GRAPHEXIA_ALGO_KRUSKAL__HPP_

#include <Graphexia/Graph.hpp>
#include <unordered_set>

namespace gpx {
    struct KruskalState {
        std::vector<usize> sortedEdges;
        std::vector<std::unordered_set<usize>> indirectConnections; 
        std::vector<usize> connections;
        usize current;

        std::vector<usize> result;
    };

    KruskalState SetupKruskal(const gpx::Graph& graph);
    bool IterateKruskal(const gpx::Graph& graph, KruskalState& state);
} // namespace gpx

#endif
