#ifndef _GRAPHEXIA_GRAPHVIEWRENDERER__HPP_
#define _GRAPHEXIA_GRAPHVIEWRENDERER__HPP_

#include "Core.hpp"

#include <cmath>
#include <concepts>
#include <numbers>
#include <random>

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

template<std::uniform_random_bit_generator Generator>
struct RandomGraphViewRenderer {
    constexpr RandomGraphViewRenderer(f32x2 offset, u32 radius, Generator generator)
        : offset(offset), radius(radius), generator(generator) {}

    f32x2 Render(usize) {
        f32 halfMax = Generator::max() / 2.f;
        f32 rX = ((this->generator() - halfMax) / halfMax) * radius;
        f32 rY = ((this->generator() - halfMax) / halfMax) * radius;
        return { (this->offset.x + rX), (this->offset.y + rY) };
    }

    private:
        f32x2 offset;
        u32 radius;
        Generator generator;
};
#endif
