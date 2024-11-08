#ifndef _GRAPHEXIA_APP_CORE__HPP_
#define _GRAPHEXIA_APP_CORE__HPP_

#include <Graphexia/Core.hpp>

enum class SelectionType : u8 {
    None = 0,
    VertexSelected = 1 << 1,
    EdgeSelected = 1 << 2,

    DrawingEdge = 1 << 6,
    DeletionRequest = 1 << 7,

    VertexDrawingEdge = VertexSelected | DrawingEdge,
    VertexDeletionRequest = VertexSelected | DeletionRequest,
};

inline SelectionType operator|(SelectionType a, SelectionType b) { return static_cast<SelectionType>(static_cast<u8>(a) | static_cast<u8>(b)); }
inline SelectionType operator&(SelectionType a, SelectionType b) { return static_cast<SelectionType>(static_cast<u8>(a) & static_cast<u8>(b)); }

struct f32x2 {
    f32 x, y; 
};

enum class GraphexiaMode : u8 {
    EditVertices,
    EditEdges,
};

#endif
