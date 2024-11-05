#include <Graphexia/graphtypes.hpp>
#include <cmath>

namespace gpx {
    void RenderKComplete(Graph &graph, usize k) {
        std::vector<usize> vertices(k);

        f32 angleIncrement = 2 * std::numbers::pi / k;
        f32 currentAngle = 0;
        for (usize i = 0; i < k; ++i) {
            i16 x = static_cast<i16>(100 * std::cos(currentAngle));
            i16 y = static_cast<i16>(100 * std::sin(currentAngle));

            vertices[i] = graph.AddVertex(x, y);
            currentAngle += angleIncrement;
        }

        for (usize i = 0; i < k; ++i) {
            usize fromId = vertices[i];

            for (usize j = i + 1; j < k; ++j) {
                usize toId = vertices[j];

                graph.AddEdge(fromId, toId);
            }
        }
    }
} // namespace gpx
