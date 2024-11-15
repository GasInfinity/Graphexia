#include "StaticTextureBatch.hpp"

namespace detail {
    sg_sampler SharedNonFilteringSampler = {};
    usize ShaderNonFilteringSamplerRef = 0;
} // namespace detail

