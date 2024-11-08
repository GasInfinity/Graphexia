#ifndef _GRAPHEXIA_APP_GRAPHEXIA__HPP_
#define _GRAPHEXIA_APP_GRAPHEXIA__HPP_

#include <Graphexia/GraphView.hpp>

#include "Core.hpp"
#include "GraphexiaRenderer.hpp"

#include <sokol/sokol_app.h>
#include <nuklear/nuklear.h>


class Graphexia final {
public:
    Graphexia(); 
    
    void Init();
    void Update(nk_context* ctx);
    void Render();

    void Event(const sapp_event* event);

    f32x2 ScreenToWorld(f32x2 position);
private:
    void ChangeMode(GraphexiaMode mode);

    gpx::GraphView view;
    GraphexiaRenderer renderer;

    GraphexiaMode mode;
    SelectionType selectionType;
    usize selectedId;
    f32x2 selectedVertexMouseOffset;

    f32x2 cameraPosition;
    f32 cameraZoom;

    bool movingCamera;
    bool currentlyDraggingVertex;
    f32x2 currentMouseWorldPosition;

    i32 savedSelectedRadius;
    i32 savedSelectedKComplete;
};

#endif
