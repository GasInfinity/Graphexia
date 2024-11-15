#ifndef _GRAPHEXIA_APP_CORE__HPP_
#define _GRAPHEXIA_APP_CORE__HPP_

#include <Graphexia/Core.hpp>

struct u32x2 {
    u32 x, y;
};

struct f32x2 {
    f32 x, y; 
};

struct u8x4 {
    u8 x, y, z, w;
};

enum class SelectionType : u8 {
    None = 0,
    VertexSelected = 1 << 1,
    EdgeSelected = 1 << 2,

    DrawingEdge = 1 << 6,
    DeletionRequest = 1 << 7,

    VertexDrawingEdge = VertexSelected | DrawingEdge,
    VertexDeletionRequest = VertexSelected | DeletionRequest,
    EdgeDeletionRequest = EdgeSelected | DeletionRequest,
};

inline SelectionType operator|(SelectionType a, SelectionType b) { return static_cast<SelectionType>(static_cast<u8>(a) | static_cast<u8>(b)); }
inline SelectionType operator&(SelectionType a, SelectionType b) { return static_cast<SelectionType>(static_cast<u8>(a) & static_cast<u8>(b)); }

enum class GraphexiaMode : u8 {
    EditVertices,
    EditEdges,
};

constexpr u8x4 Rgba8(u32 value) {
    return { static_cast<u8>(value >> 24), static_cast<u8>((value >> 16) & 0xFF), static_cast<u8>((value >> 8) & 0xFF), static_cast<u8>(value & 0xFF) };
}

#endif
