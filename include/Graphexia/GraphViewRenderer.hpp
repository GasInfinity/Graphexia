#ifndef _GRAPHEXIA_GRAPHVIEWRENDERER__HPP_
#define _GRAPHEXIA_GRAPHVIEWRENDERER__HPP_

#include <Graphexia/Core.hpp>
#include <cmath>
#include <concepts>
#include <numbers>

namespace gpx {
    template<typename T>
    concept GraphViewRenderer = requires(T renderer) {
        { renderer.Render(usize()) } -> std::same_as<i16x2>;
    };

    struct CircularGraphViewRenderer {
        constexpr CircularGraphViewRenderer(i16x2 offset, usize radius, usize vertices)
            : offset(offset), radius(radius), angleIncrement(2 * std::numbers::pi / vertices) {}

        i16x2 Render(usize vertex) {
            f32 angle = angleIncrement * vertex;
            f32 sin = std::sin(angle);
            f32 cos = std::cos(angle);

            return { static_cast<i16>(this->offset.x + cos * radius), static_cast<i16>(this->offset.y + sin * radius) };
        }

        private:
            i16x2 offset;
            u16 radius;
            f32 angleIncrement;
    };
}

#endif
