#include <Graphexia/GraphTypes.hpp>
#include <cmath>

namespace gpx {
    Graph CreateKComplete(usize k) {
        Graph kCompleteGraph(k);

        for (usize i = 0; i < k; ++i) {
            usize fromId = i;

            for (usize j = i + 1; j < k; ++j) {
                usize toId = j;

                kCompleteGraph.AddEdge(fromId, toId);
            }
        }

        return kCompleteGraph;
    }
} // namespace gpx
