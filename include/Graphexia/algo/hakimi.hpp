#ifndef _GRAPHEXIA_ALGO_HAKIMI__HPP_
#define _GRAPHEXIA_ALGO_HAKIMI__HPP_

#include <Graphexia/graph.hpp>
#include <span>

namespace gpx {
    bool IsValidSequence(std::span<usize> sequence);
    void RenderSequence(Graph& graph, std::span<usize> sequence);
} // namespace gpx
#endif
