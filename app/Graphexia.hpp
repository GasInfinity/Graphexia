#ifndef _GRAPHEXIA_APP_GRAPHEXIA__HPP_
#define _GRAPHEXIA_APP_GRAPHEXIA__HPP_

#include "GraphView.hpp"

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
private:
    void ChangeMode(GraphexiaMode mode);

    GraphView view;
    GraphexiaRenderer renderer;

    GraphexiaMode mode;
    SelectionType selectionType;
    usize selectedId;
    f32x2 selectedVertexMouseOffset;

    bool movingCamera;
    bool currentlyDraggingVertex;
    f32x2 currentMouseWorldPosition;

    i32 savedSelectedRadius;
    i32 savedSelectedKComplete;
};

#endif
