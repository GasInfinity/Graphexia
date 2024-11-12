#include "GraphexiaRenderer.hpp"

#include "Core.hpp"
#include "GraphView.hpp"
#include <cstring>
#include <vector>

// FIXME: Check if failed to create shader and pipeline
void GraphexiaRenderer::Init(u32x2 viewport) {
    this->viewport = viewport;
    this->cameraPosition = {};
    this->cameraZoom = 1;
    this->UpdateGlobalData();

    this->gVtxShader = sg_make_shader(Graphexia_GraphVtx_shader_desc(sg_query_backend()));
    this->gEdgesShader = sg_make_shader(Graphexia_GraphEdge_shader_desc(sg_query_backend()));
    
    sg_pipeline_desc gVtxPipelineDesc{};
    gVtxPipelineDesc.label = "Graph Vertices Pipeline";
    gVtxPipelineDesc.shader = this->gVtxShader; 
    gVtxPipelineDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    gVtxPipelineDesc.colors[0].blend = {
        .enabled = true,
        .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
        .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .op_rgb = SG_BLENDOP_ADD,
        .src_factor_alpha = SG_BLENDFACTOR_ONE,
        .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .op_alpha = SG_BLENDOP_ADD
    };
    this->gVtxPipeline = sg_make_pipeline(&gVtxPipelineDesc);

    sg_pipeline_desc gEdgesPipelineDesc{};
    gEdgesPipelineDesc.label = "Graph Edges Pipeline";
    gEdgesPipelineDesc.shader = this->gEdgesShader; 
    gEdgesPipelineDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    gEdgesPipelineDesc.colors[0].blend = {
        .enabled = true,
        .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
        .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .op_rgb = SG_BLENDOP_ADD,
        .src_factor_alpha = SG_BLENDFACTOR_ONE,
        .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .op_alpha = SG_BLENDOP_ADD
    };
    this->gEdgesPipeline = sg_make_pipeline(&gEdgesPipelineDesc);

    sg_image_desc gVtxImageDesc{};
    gVtxImageDesc.label = "Graph Vertices Batched Data";
    gVtxImageDesc.width = BatchedTextureVtxDimensions;
    gVtxImageDesc.height = BatchedTextureVtxDimensions;
    gVtxImageDesc.pixel_format = SG_PIXELFORMAT_RGBA32UI;
    gVtxImageDesc.usage = SG_USAGE_STREAM;
    gVtxImageDesc.num_mipmaps = 1;
    this->gVtxImage = sg_make_image(&gVtxImageDesc);

    sg_image_desc gEdgesImageDesc{};
    gEdgesImageDesc.label = "Graph Edges Batched Data";
    gEdgesImageDesc.width = BatchedTextureEdgeDimensions;
    gEdgesImageDesc.height = BatchedTextureEdgeDimensions;
    gEdgesImageDesc.pixel_format = SG_PIXELFORMAT_RGBA32UI;
    gEdgesImageDesc.usage = SG_USAGE_STREAM;
    gEdgesImageDesc.num_mipmaps = 1;
    this->gEdgesImage = sg_make_image(&gEdgesImageDesc);

    sg_sampler_desc gDataSamplerDesc{};
    gDataSamplerDesc.label = "Graph Batched Data Sampler";
    gDataSamplerDesc.min_filter = SG_FILTER_NEAREST;
    gDataSamplerDesc.mag_filter = SG_FILTER_NEAREST;
    gDataSamplerDesc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    gDataSamplerDesc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    this->gImageSampler = sg_make_sampler(&gDataSamplerDesc);

    sg_bindings gVtxBindings{};
    gVtxBindings.images[IMG_VtxBatchDataTex].id = this->gVtxImage.id;
    gVtxBindings.samplers[SMP_BatchDataSmp].id = this->gImageSampler.id;
    this->gVtxBindings = gVtxBindings;

    sg_bindings gEdgesBindings{};
    gEdgesBindings.images[IMG_VtxBatchDataTex].id = this->gVtxImage.id;
    gEdgesBindings.images[IMG_EdgeBatchDataTex].id = this->gEdgesImage.id;
    gEdgesBindings.samplers[SMP_BatchDataSmp].id = this->gImageSampler.id;
    this->gEdgesBindings = gEdgesBindings;
}

void GraphexiaRenderer::ReconstructView(const GraphView& view) {
    const gpx::Graph& graph = view.GetGraph();

    this->graphDirty = true;
    if(!graph.Vertices()) {
        this->gVtxBatched = 0;
        this->gEdgesBatched = 0;
        return;
    }

    const std::vector<Vertex>& vertexViews = view.Vertices();
    for (usize i = 0; i < vertexViews.size(); ++i) { 
        const Vertex& vertex = vertexViews[i];

        this->gVtxData[i] = {vertex.position, vertex.size, 0xFFFFFFFF};
    }
    this->gVtxBatched = vertexViews.size();

    const std::vector<gpx::Edge>& edges = graph.Edges();
    for (usize i = 0; i < edges.size(); ++i) { 
        const gpx::Edge& edge = edges[i];

        this->gEdgeData[i] = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), static_cast<f32>(1), 0xFFFFFFFF};
    }

    this->gEdgesBatched = edges.size();
}

void GraphexiaRenderer::ReconstructEdges(const std::vector<gpx::Edge>& edges) {
    for (usize i = 0; i < edges.size(); ++i) { 
        const gpx::Edge& edge = edges[i];

        this->gEdgeData[i] = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), static_cast<f32>(1), 0xFFFFFFFF};
    }

    this->gEdgesBatched = edges.size();
}

void GraphexiaRenderer::AddVertex(const Vertex& view) {
    this->gVtxData[this->gVtxBatched++] = {view.position, view.size, 0xFFFFFFFF};
    this->graphDirty = true;
}

void GraphexiaRenderer::AddEdge(const gpx::Edge& edge) {
    // HACK: Assume less than UINT_MAX
    this->gEdgeData[this->gEdgesBatched++] = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), 1, 0xFFFFFFFF};
    this->graphDirty = true;
}

void GraphexiaRenderer::UpdateVertexPosition(usize id, f32x2 position) {
    this->gVtxData[id].position = position;
    this->graphDirty = true;
}

void GraphexiaRenderer::UpdateVertexColor(usize id, u32 color) {
    this->gVtxData[id].color = color;
    this->graphDirty = true;
}

void GraphexiaRenderer::DeleteVertex(usize id) {
    this->graphDirty = true;
    if(id != this->gVtxBatched - 1) {
        memmove(&this->gVtxData[id], &this->gVtxData[id+1], (this->gVtxBatched - id - 1) * sizeof(BatchedVertex));
    }

    --this->gVtxBatched;
    this->graphDirty = true;
}

void GraphexiaRenderer::Render(const GraphView& view, const SelectionType selectionType, const usize selectedId, const f32x2 worldMousePosition) {
    if(this->gVtxBatched == 0) {
        return;
    }

    if(this->graphDirty) {
        sg_image_data gVtxImageData{};
        gVtxImageData.subimage[0][0] = { .ptr = this->gVtxData, .size = BatchedTextureVtxDimensions * BatchedTextureVtxDimensions * sizeof(BatchedVertex) };
        sg_update_image(this->gVtxImage, &gVtxImageData);

        sg_image_data gEdgesImageData{};
        gEdgesImageData.subimage[0][0] = { .ptr = this->gEdgeData, .size = BatchedTextureEdgeDimensions * BatchedTextureEdgeDimensions * sizeof(BatchedEdge) };
        sg_update_image(this->gEdgesImage, &gEdgesImageData);
    }

    sg_apply_pipeline(this->gEdgesPipeline);
    sg_apply_bindings(this->gEdgesBindings);
    sg_apply_uniforms(UB_GlobalData, SG_RANGE(this->gGlobalData));
    sg_draw(0, this->gEdgesBatched * 6, 1);

    sg_apply_pipeline(this->gVtxPipeline);
    sg_apply_bindings(this->gVtxBindings);
    sg_apply_uniforms(UB_GlobalData, SG_RANGE(this->gGlobalData)); 
    sg_draw(0, this->gVtxBatched * 6, 1);
}

void GraphexiaRenderer::SetCameraZoom(f32 zoom) {
    if(this->cameraZoom == zoom) {
        return;
    }

    this->cameraZoom = zoom;
    this->UpdateGlobalData();
}

void GraphexiaRenderer::SetCameraPosition(f32x2 position) {
    if(this->cameraPosition.x == position.x
    && this->cameraPosition.y == position.y) {
        return;
    }

    this->cameraPosition = position;
    this->UpdateGlobalData();
}

void GraphexiaRenderer::SetViewport(u32x2 viewport) {
    if(this->viewport.x == viewport.x
    && this->viewport.y == viewport.y) {
        return;
    }

    this->viewport = viewport;
    this->UpdateGlobalData();
}

void GraphexiaRenderer::UpdateGlobalData() {
    f32 vScaleX = 2.0f / this->viewport.x;
    f32 vScaleY = -2.0f / this->viewport.y;

    f32 viewportMiddleX = this->viewport.x / 2.f;
    f32 viewportMiddleY = this->viewport.y / 2.f;
    this->gGlobalData = {
        {vScaleX * this->cameraZoom, 0, 0, vScaleY * this->cameraZoom},
        {vScaleX * (this->cameraPosition.x + viewportMiddleX) - 1, vScaleY * (this->cameraPosition.y + viewportMiddleY) + 1, 0, 0}
    };
}

f32x2 GraphexiaRenderer::ScreenToWorld(f32x2 screenPosition) const {
    f32 mW = this->viewport.x/2.f, mH = this->viewport.y/2.f;
    return { (screenPosition.x - mW - this->cameraPosition.x) / this->cameraZoom, (screenPosition.y - mH - this->cameraPosition.y) / this->cameraZoom };
}
