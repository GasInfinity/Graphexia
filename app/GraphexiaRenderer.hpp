#ifndef _GRAPHEXIA_APP_GRAPHEXIARENDERER__HPP_
#define _GRAPHEXIA_APP_GRAPHEXIARENDERER__HPP_

#include <Graphexia/GraphView.hpp>
#include "Core.hpp"

#include <sokol/sokol_gfx.h>
#include "sokol/sokol_gp.h"

class GraphexiaRenderer final {
public:
    void Init();
    void RenderView(const gpx::GraphView& view, const SelectionType selectionType, const usize selectedId, const f32x2 worldMousePosition);

private:
    sg_shader vtxShader;
    sg_pipeline vtxPipeline;
};
#endif
