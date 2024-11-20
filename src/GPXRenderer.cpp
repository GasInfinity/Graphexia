#include "GPXRenderer.hpp"

#include "Core.hpp"
#include "GraphView.hpp"
#include "GPXFontRenderer.hpp"

#include <cstring>
#include <format>
#include <string>
#include <vector>

// FIXME: Check if failed to create shader and pipeline
void GPXRenderer::Init(u32x2 viewport) {
    this->viewport = viewport;
    this->cameraPosition = {};
    this->cameraZoom = 1;
    this->UpdateGlobalData();

    this->batchedVertices = StaticTextureBatch<BatchedVtxDimensions, BatchedVertex, IMG_VtxBatchDataTex, SMP_BatchDataSmp>::Create(
        Graphexia_GraphVtx_shader_desc(sg_query_backend()), {}
    ).value();

    sg_bindings preEdgesBindings{};
    preEdgesBindings.images[IMG_VtxBatchDataTex] = this->batchedVertices.BatchedImage();

    this->batchedEdges = StaticTextureBatch<BatchedEdgeDimensions, BatchedEdge, IMG_EdgeBatchDataTex, SMP_BatchDataSmp>::Create(
        Graphexia_GraphEdge_shader_desc(sg_query_backend()), preEdgesBindings 
    ).value();

    this->fontRenderer.Init(reinterpret_cast<Graphexia_GlobalFontData_t*>(&this->gGlobalData));
    this->fontRenderer.DrawText({-32.5, -6}, 12, "Graphexia");
}

void GPXRenderer::ReconstructView(const GraphView& view) {
    const gpx::Graph& graph = view.GetGraph();

    this->weightsDirty = true;
    if(!graph.Vertices()) {
        this->batchedVertices.ClearBatched();
        this->batchedEdges.ClearBatched();
        return;
    }

    auto& batchedVtxData = this->batchedVertices.Data();
    const std::vector<Vertex>& vertexViews = view.Vertices();
    for (usize i = 0; i < vertexViews.size(); ++i) { 
        const Vertex& vertex = vertexViews[i];

        batchedVtxData.at(i) = {vertex.position, vertex.size, Rgba8(0xFFFFFFFF)};
    }
    this->batchedVertices.SetBatchedCount(vertexViews.size());

    auto& batchedEdgeData = this->batchedEdges.Data();
    const std::vector<gpx::Edge>& edges = graph.Edges();
    for (usize i = 0; i < edges.size(); ++i) { 
        const gpx::Edge& edge = edges[i];

        batchedEdgeData.at(i) = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), static_cast<f32>(1), Rgba8(0xFFFFFFFF)};
    }

    this->batchedEdges.SetBatchedCount(edges.size());
}

void GPXRenderer::ReconstructEdges(const std::vector<gpx::Edge>& edges) {
    auto& batchedEdgeData = this->batchedEdges.Data();
    for (usize i = 0; i < edges.size(); ++i) { 
        const gpx::Edge& edge = edges[i];

        batchedEdgeData.at(i) = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), static_cast<f32>(1), Rgba8(0xFFFFFFFF)};
    }

    this->batchedEdges.SetBatchedCount(edges.size());
    this->weightsDirty = true;
}

void GPXRenderer::AddVertex(const Vertex& view) {
    this->batchedVertices.Data().at(this->batchedVertices.BatchedCount()) = {view.position, view.size, Rgba8(0xFFFFFFFF)};
    this->batchedVertices.SetBatchedCount(this->batchedVertices.BatchedCount() + 1);
}

void GPXRenderer::AddEdge(const gpx::Edge& edge) {
    // HACK: Assume less than UINT_MAX
    this->batchedEdges.Data().at(this->batchedEdges.BatchedCount()) = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), 1, Rgba8(0xFFFFFFFF)};
    this->batchedEdges.SetBatchedCount(this->batchedEdges.BatchedCount() + 1);
    this->weightsDirty = true;
}

void GPXRenderer::UpdateVertexPosition(usize id, f32x2 position) {
    this->batchedVertices.Data().at(id).position = position;
    this->batchedVertices.FlagDirty();
    this->weightsDirty = true;
}

void GPXRenderer::UpdateVertexColor(usize id, u8x4 color) {
    this->batchedVertices.Data().at(id).color = color;
    this->batchedVertices.FlagDirty();
}

void GPXRenderer::UpdateEdgeColor(usize id, u8x4 color) {
    this->batchedEdges.Data().at(id).color = color;
    this->batchedEdges.FlagDirty();
}

void GPXRenderer::UpdateWeights() {
    this->weightsDirty = true;
}

void GPXRenderer::EraseVertex(usize id) {
    const usize newSize = this->batchedVertices.BatchedCount() - 1;

    auto& batchedVtxData = this->batchedVertices.Data();
    if(id != newSize) {
        memmove(&batchedVtxData.at(id), &batchedVtxData.at(id + 1), (newSize - id) * sizeof(BatchedVertex));
    }

    this->batchedVertices.SetBatchedCount(newSize);
}

void GPXRenderer::EraseEdge(usize id) {
    const usize newSize = this->batchedEdges.BatchedCount() - 1;

    auto& batchedEdgeData = this->batchedEdges.Data();
    if(id != newSize) {
        memmove(&batchedEdgeData.at(id), &batchedEdgeData.at(id + 1), (newSize - id) * sizeof(BatchedEdge));
    }

    this->batchedEdges.SetBatchedCount(newSize);
    this->weightsDirty = true;
}

void GPXRenderer::Update(const GraphView& view) {
    this->batchedEdges.Update();
    this->batchedVertices.Update();

    if(this->weightsDirty) {
        this->fontRenderer.Clear();
        this->fontRenderer.DrawText({-32.5, -6}, 12, "Graphexia");

        std::string weightString;
        weightString.reserve(20);
        for (const gpx::Edge& edge : view.GetGraph().Edges()) {
            const Vertex& from = view.Vertices()[edge.fromId]; 
            const Vertex& to = view.Vertices()[edge.toId]; 
            
            f32 x = (from.position.x + to.position.x) / 2.f;
            f32 y = (from.position.y + to.position.y) / 2.f - 3.f;
            weightString.clear();
            std::format_to(std::back_inserter(weightString), "{:.2f}", edge.weight);
            this->fontRenderer.DrawText({x, y}, 8, weightString);
        }

        this->weightsDirty = false;
    } 

    this->fontRenderer.Update();
}

void GPXRenderer::Render() {
    this->fontRenderer.Render();

    auto globals = this->gGlobalData;
    this->batchedEdges.Render([&globals]() {
        sg_apply_uniforms(UB_GlobalGraphData, SG_RANGE(globals));
    });

    this->batchedVertices.Render([&globals]() {
        sg_apply_uniforms(UB_GlobalGraphData, SG_RANGE(globals));
    });
}

void GPXRenderer::SetCameraZoom(f32 zoom) {
    if(this->cameraZoom == zoom) {
        return;
    }

    this->cameraZoom = zoom;
    this->UpdateGlobalData();
}

void GPXRenderer::SetCameraPosition(f32x2 position) {
    if(this->cameraPosition.x == position.x
    && this->cameraPosition.y == position.y) {
        return;
    }

    this->cameraPosition = position;
    this->UpdateGlobalData();
}

void GPXRenderer::SetViewport(u32x2 viewport) {
    if(this->viewport.x == viewport.x
    && this->viewport.y == viewport.y) {
        return;
    }

    this->viewport = viewport;
    this->UpdateGlobalData();
}

void GPXRenderer::UpdateGlobalData() {
    f32 vScaleX = 2.0f / this->viewport.x;
    f32 vScaleY = -2.0f / this->viewport.y;

    f32 viewportMiddleX = this->viewport.x / 2.f;
    f32 viewportMiddleY = this->viewport.y / 2.f;
    this->gGlobalData = {
        {vScaleX * this->cameraZoom, 0, 0, vScaleY * this->cameraZoom},
        {vScaleX * (this->cameraPosition.x + viewportMiddleX) - 1, vScaleY * (this->cameraPosition.y + viewportMiddleY) + 1, 0, 0}
    };
}

f32x2 GPXRenderer::ScreenToWorld(f32x2 screenPosition) const {
    f32 mW = this->viewport.x/2.f, mH = this->viewport.y/2.f;
    return { (screenPosition.x - mW - this->cameraPosition.x) / this->cameraZoom, (screenPosition.y - mH - this->cameraPosition.y) / this->cameraZoom };
}
