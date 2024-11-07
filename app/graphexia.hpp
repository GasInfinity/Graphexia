#ifndef _GRAPHEXIA_APP_GRAPHEXIA__HPP_
#define _GRAPHEXIA_APP_GRAPHEXIA__HPP_

#include <Graphexia/graphview.hpp>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include "sokol_gp.h"
#include <nuklear.h>

enum class GraphMode : u8 {
    EditVertices,
    DrawEdges,
};

struct Vec2 {
    f32 x, y; 
};

class Graphexia final {
public:
    Graphexia(); 
    
    void Init();
    void Update(nk_context* ctx);
    void Render();

    void Event(const sapp_event* event);

    Vec2 ScreenToWorld(Vec2 position);
private:
    void ChangeMode(GraphMode mode);

    gpx::GraphView view;

    GraphMode mode;
    usize selectedVertex;
    Vec2 selectedVertexMouseOffset;

    Vec2 cameraPosition;
    f32 cameraZoom;

    bool movingCamera;
    bool currentlyDraggingVertex;
    Vec2 currentMouseWorldPosition;

    i32 savedSelectedRadius;
    i32 savedSelectedKComplete;
};

#endif
