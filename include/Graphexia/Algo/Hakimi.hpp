#ifndef _GRAPHEXIA_ALGO_HAKIMI__HPP_
#define _GRAPHEXIA_ALGO_HAKIMI__HPP_

#include <Graphexia/Graph.hpp>
#include <span>

namespace gpx {
    bool IsGraphicSequence(std::span<usize> sequence);
    Graph CreateFromGraphicSequence(std::span<usize> sequence);
} // namespace gpx
#endif
