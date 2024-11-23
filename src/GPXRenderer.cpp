#include "GPXRenderer.hpp"

#include "Core.hpp"
#include "GraphView.hpp"
#include "GPXFontRenderer.hpp"
#include "Util/EasingTask.hpp"

#include <cstdlib>
#include <cstring>
#include <format>
#include <string>
#include <vector>

void GPXRenderer::ReconstructView(const GraphView& view) {
    const gpx::Graph& graph = view.GetGraph();

    if(!graph.Vertices()) {
        m.batchedVertices.ClearBatched();
        m.batchedEdges.ClearBatched();
        return;
    }

    auto& batchedVtxData = m.batchedVertices.Data();
    const std::vector<Vertex>& vertexViews = view.Vertices();

    m.vertexSizeAnimations.clear();
    m.vertexSizeAnimations.reserve(vertexViews.size());
    m.vertexSizeAnimations.resize(vertexViews.size());

    for (usize i = 0; i < vertexViews.size(); ++i) { 
        const Vertex& vertex = vertexViews[i];

        batchedVtxData.at(i) = {vertex.position, 0.f, Rgba8(0xFFFFFFFF)};
        m.vertexSizeAnimations.at(i) = AnimationTask(i, EasingTask(0.f, 3.f, 0.4f, Easing::OutBack10));
    }
    m.batchedVertices.SetBatchedCount(vertexViews.size());

    auto& batchedEdgeData = m.batchedEdges.Data();
    const std::vector<gpx::Edge>& edges = graph.Edges();

    m.edgeSizeAnimations.clear();
    m.edgeSizeAnimations.reserve(edges.size());
    m.edgeSizeAnimations.resize(edges.size());

    for (usize i = 0; i < edges.size(); ++i) { 
        const gpx::Edge& edge = edges[i];

        batchedEdgeData.at(i) = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), static_cast<f32>(0.f), Rgba8(0xFFFFFFFF)};
        m.edgeSizeAnimations.at(i) = AnimationTask(i, EasingTask(0.f, 0.5f, 0.4f, Easing::OutBack10));
    }

    m.batchedEdges.SetBatchedCount(edges.size());
}

void GPXRenderer::ReconstructEdges(const std::vector<gpx::Edge>& edges) {
    auto& batchedEdgeData = m.batchedEdges.Data();
    for (usize i = 0; i < edges.size(); ++i) { 
        const gpx::Edge& edge = edges[i];

        batchedEdgeData.at(i) = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), static_cast<f32>(0.5f), Rgba8(0xFFFFFFFF)};
    }

    m.batchedEdges.SetBatchedCount(edges.size());
}

void GPXRenderer::AddVertex(const Vertex& view) {
    usize id = m.batchedVertices.BatchedCount();

    m.vertexSizeAnimations.push_back(AnimationTask(id, EasingTask(0.f, 3.f, 0.4f, Easing::OutBack10)));
    m.batchedVertices.Data().at(id) = {view.position, 0.f, Rgba8(0xFFFFFFFF)};
    m.batchedVertices.SetBatchedCount(id + 1);
}

void GPXRenderer::AddEdge(const gpx::Edge& edge) {
    usize id = m.batchedEdges.BatchedCount();

    // HACK: Assume less than UINT_MAX
    m.edgeSizeAnimations.push_back(AnimationTask(id, EasingTask(0.f, 1.f, 0.4f, Easing::OutBack10)));
    m.batchedEdges.Data().at(id) = {static_cast<u32>(edge.fromId), static_cast<u32>(edge.toId), 1, Rgba8(0xFFFFFFFF)};
    m.batchedEdges.SetBatchedCount(id + 1);
}

void GPXRenderer::UpdateVertexPosition(usize id, f32x2 position) {
    m.batchedVertices.Data().at(id).position = position;
    m.batchedVertices.FlagDirty();
}

void GPXRenderer::UpdateVertexColor(usize id, u8x4 color) {
    m.batchedVertices.Data().at(id).color = color;
    m.batchedVertices.FlagDirty();
}

void GPXRenderer::UpdateEdgeColor(usize id, u8x4 color) {
    m.batchedEdges.Data().at(id).color = color;
    m.batchedEdges.FlagDirty();
}

void GPXRenderer::EraseVertex(usize id) {
    const usize newSize = m.batchedVertices.BatchedCount() - 1;

    auto& batchedVtxData = m.batchedVertices.Data();
    if(id != newSize) {
        memmove(&batchedVtxData.at(id), &batchedVtxData.at(id + 1), (newSize - id) * sizeof(ShaderVertex));
    }
    
    // FIXME: Remove animation if playing
    m.batchedVertices.SetBatchedCount(newSize);
}

void GPXRenderer::EraseEdge(usize id) {
    const usize newSize = m.batchedEdges.BatchedCount() - 1;

    auto& batchedEdgeData = m.batchedEdges.Data();
    if(id != newSize) {
        memmove(&batchedEdgeData.at(id), &batchedEdgeData.at(id + 1), (newSize - id) * sizeof(ShaderEdge));
    }

    m.batchedEdges.SetBatchedCount(newSize);
}

void GPXRenderer::Update(f32 dt) {
    this->UpdateAnimations(dt);

    m.batchedEdges.Update();
    m.batchedVertices.Update();
    m.fontRenderer.Update();
}

void GPXRenderer::UpdateAnimations(f32 dt) {
    for (usize i = m.vertexSizeAnimations.size(); i > 0; --i) {
        usize index = i - 1;

        AnimationTask<f32>& sizeAnimation = m.vertexSizeAnimations[index];
        EasingTask<f32>& sizeEasing = sizeAnimation.Easing();

        if(sizeEasing.Finished()) {
            m.vertexSizeAnimations.erase(m.vertexSizeAnimations.begin() + index);
            continue;
        }

        f32 newSize = sizeEasing.Update(dt);

        m.batchedVertices.Data().at(sizeAnimation.Id()).size = newSize;
        m.batchedVertices.FlagDirty();
    }

    for (usize i = m.edgeSizeAnimations.size(); i > 0; --i) {
        usize index = i - 1;

        AnimationTask<f32>& sizeAnimation = m.edgeSizeAnimations[index];
        EasingTask<f32>& sizeEasing = sizeAnimation.Easing();

        if(sizeEasing.Finished()) {
            m.edgeSizeAnimations.erase(m.edgeSizeAnimations.begin() + index);
            continue;
        }

        f32 newSize = sizeEasing.Update(dt);

        m.batchedEdges.Data().at(sizeAnimation.Id()).size = newSize;
        m.batchedEdges.FlagDirty();
    }
}

void GPXRenderer::Render() {
    m.fontRenderer.Render();

    auto globals = m.graphShaderData;

    if(m.cameraZoom > 1.f) {
        m.batchedEdges.Render([&globals]() {
            sg_apply_uniforms(UB_GlobalGraphData, SG_RANGE(globals));
        });
    }

    m.batchedVertices.Render([&globals]() {
        sg_apply_uniforms(UB_GlobalGraphData, SG_RANGE(globals));
    });
}

void GPXRenderer::SetCameraZoom(f32 zoom) {
    if(m.cameraZoom == zoom) {
        return;
    }

    m.cameraZoom = zoom;
    this->UpdateRendererData();
}

void GPXRenderer::SetCameraPosition(f32x2 position) {
    if(m.cameraPosition.x == position.x
    && m.cameraPosition.y == position.y) {
        return;
    }

    m.cameraPosition = position;
    this->UpdateRendererData();
}

void GPXRenderer::SetViewport(u32x2 viewport) {
    if(m.viewport.x == viewport.x
    && m.viewport.y == viewport.y) {
        return;
    }

    m.viewport = viewport;
    this->UpdateRendererData();
}

void GPXRenderer::UpdateRendererData() {
    f32 vScaleX = 2.0f / m.viewport.x;
    f32 vScaleY = -2.0f / m.viewport.y;

    f32 viewportMiddleX = m.viewport.x / 2.f;
    f32 viewportMiddleY = m.viewport.y / 2.f;
    
    // The matrix is column-major!
    m.graphShaderData = {
        vScaleX * m.cameraZoom, 0,
        0, vScaleY * m.cameraZoom,
        vScaleX * (m.cameraPosition.x + viewportMiddleX) - 1, vScaleY * (m.cameraPosition.y + viewportMiddleY) + 1,
        std::clamp(m.cameraZoom - 1.f, 0.f, .5f) * 2.f, 0
    };

    m.fontRenderer.UpdateFontShaderData(GPXFontShaderData{
        m.graphShaderData.m00, m.graphShaderData.m10, m.graphShaderData.m20,
        m.graphShaderData.m01, m.graphShaderData.m11, m.graphShaderData.m21,
        0.f, 0.f
    });
}

f32x2 GPXRenderer::ScreenToWorld(f32x2 screenPosition) const {
    f32 mW = m.viewport.x/2.f, mH = m.viewport.y/2.f;
    return { (screenPosition.x - mW - m.cameraPosition.x) / m.cameraZoom, (screenPosition.y - mH - m.cameraPosition.y) / m.cameraZoom };
}

std::optional<GPXRenderer> GPXRenderer::Create(u32x2 initialViewport) {
    auto batchedVertices = StaticTextureBatch<BatchedVtxDimensions, ShaderVertex, IMG_VtxBatchDataTex, SMP_BatchDataSmp>::Create(
        Graphexia_GraphVtx_shader_desc(sg_query_backend()), {}
    );

    if(!batchedVertices) {
        return std::nullopt;
    }

    sg_bindings preEdgesBindings{};
    preEdgesBindings.images[IMG_VtxBatchDataTex] = batchedVertices->BatchedImage();

    auto batchedEdges = StaticTextureBatch<BatchedEdgeDimensions, ShaderEdge, IMG_EdgeBatchDataTex, SMP_BatchDataSmp>::Create(
        Graphexia_GraphEdge_shader_desc(sg_query_backend()), preEdgesBindings 
    );

    if(!batchedEdges) {
        return std::nullopt;
    }

    std::optional<GPXFontRenderer> fontRenderer = GPXFontRenderer::Create();

    if(!fontRenderer) {
        return std::nullopt;
    }

    fontRenderer->DrawText({-32.5, -6}, 12, "Graphexia");

    return GPXRenderer(M{
        initialViewport,
        {0.f, 0.f},
        1.f,
        {},
        std::move(*batchedVertices),
        std::move(*batchedEdges),
        std::vector<AnimationTask<f32>>(),
        std::vector<AnimationTask<f32>>(),
        std::move(*fontRenderer)
    });
}
