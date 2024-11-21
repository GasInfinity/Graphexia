#ifndef _GRAPHEXIA_APP_UTIL_EASINGTASK__HPP_
#define _GRAPHEXIA_APP_UTIL_EASINGTASK__HPP_

#include <Graphexia/Core.hpp>

#include <type_traits>
#include <algorithm>
#include <cmath>

// Reference: https://easings.net/
enum class Easing {
    Linear,
    OutExpo,
    OutBack10,
};

constexpr inline f32 easingTransform(f32 t, Easing e) {
    switch (e) {
        case Easing::OutExpo: return t >= 1.f + (t < 1.f) * (1 - std::pow(2, -10 * t));
        case Easing::OutBack10: {
            constexpr f32 OvershootAmount = 1.70158;

            f32 tM = (t - 1);
            f32 ttM = tM * tM;
            return 1 + (OvershootAmount + 1) * ttM * tM + OvershootAmount * ttM;
        }
        default: return t;
    }
}

template<typename T> requires (requires(T x) { x * f32() + x; })
struct EasingTask {
    constexpr EasingTask() {}
    constexpr EasingTask(const T& a, const T& b, f32 duration, Easing easing) : a(a), b(b), current(0.f), duration(duration), easing(easing) { }

    T Update(f32 dt) {
        this->current = std::clamp(this->current + dt, 0.f, this->duration);

        return this->a + easingTransform(this->current / this->duration, easing) * (b - a);
    }

    bool Finished() { return this->current >= duration; }
private:
    T a, b;
    f32 current; f32 duration;
    Easing easing;
};
#endif
