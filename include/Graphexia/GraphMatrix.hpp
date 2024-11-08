#ifndef _GRAPHEXIA_GRAPHMATRIX__HPP_
#define _GRAPHEXIA_GRAPHMATRIX__HPP_

#include <Graphexia/Graph.hpp>

namespace gpx {
    enum class IncidenceState : i8 {
        Leaves = -1,
        None = 0,
        Incident = 1
    };

    std::vector<usize> AdjacencyMatrix(const Graph& graph);
    std::vector<IncidenceState> IncidenceMatrix(const Graph& graph);
} // namespace gpx
#endif
