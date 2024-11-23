#ifndef _GRAPHEXIA_APP_GPXSHADERDATA__HPP_
#define _GRAPHEXIA_APP_GPXSHADERDATA__HPP_

#include "Core.hpp"

struct GPXShaderData {
    f32 m00, m10, m20;
    f32 m01, m11, m21;
    f32 unused0, unused1;
};

struct ShaderVertex {
    f32x2 position;
    f32 size;
    u8x4 color; // RGBA8
};

struct ShaderEdge {
    u32 fromId;
    u32 toId;
    f32 size;
    u8x4 color;
};


#endif
