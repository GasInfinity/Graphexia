#ifndef _GRAPHEXIA_GRAPHRENDERER__HPP_
#define _GRAPHEXIA_GRAPHRENDERER__HPP_

#include <Graphexia/core.hpp>
#include <cmath>
#include <concepts>
#include <numbers>

namespace gpx {
    template<typename T>
    concept GraphRenderer = requires(T renderer) {
        { renderer.Render(usize()) } -> std::same_as<Point>;
    };

    struct CircularGraphRenderer {
        constexpr CircularGraphRenderer(Point offset, usize radius, usize vertices)
            : offset(offset), radius(radius), angleIncrement(2 * std::numbers::pi / vertices) {}

        Point Render(usize vertex) {
            f32 angle = angleIncrement * vertex;
            f32 sin = std::sin(angle);
            f32 cos = std::cos(angle);

            return { static_cast<i16>(this->offset.x + cos * radius), static_cast<i16>(this->offset.y + sin * radius) };
        }

        private:
            Point offset;
            u16 radius;
            f32 angleIncrement;
    };
}

#endif
