#ifndef _GRAPHEXIA_GRAPHVIEWRENDERER__HPP_
#define _GRAPHEXIA_GRAPHVIEWRENDERER__HPP_

#include "Core.hpp"

#include <cmath>
#include <concepts>
#include <numbers>

template<typename T>
concept GraphViewRenderer = requires(T renderer) {
    { renderer.Render(usize()) } -> std::same_as<f32x2>;
};

struct CircularGraphViewRenderer {
    constexpr CircularGraphViewRenderer(f32x2 offset, u32 radius, usize vertices)
        : offset(offset), radius(radius), angleIncrement(2 * std::numbers::pi / vertices) {}

    f32x2 Render(usize vertex) {
        f32 angle = angleIncrement * vertex;
        f32 sin = std::sin(angle);
        f32 cos = std::cos(angle);

        return { (this->offset.x + cos * radius), (this->offset.y + sin * radius) };
    }

    private:
        f32x2 offset;
        u32 radius;
        f32 angleIncrement;
};

#endif
