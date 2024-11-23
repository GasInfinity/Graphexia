#ifndef _GRAPHEXIA_APP_GPXRENDERER__HPP_
#define _GRAPHEXIA_APP_GPXRENDERER__HPP_

#include "Core.hpp"
#include "GPXShaderData.hpp"
#include "GPXFontRenderer.hpp"
#include "GraphView.hpp"
#include "Render/StaticTextureBatch.hpp"
#include "Util/EasingTask.hpp"

#include <sokol/sokol_gfx.h>

#include <vector>

// autogenerated...
#include "Graph.glsl.h"

const usize BatchedVtxDimensions = 32;
const usize BatchedEdgeDimensions = 1024;

template<typename T>
struct AnimationTask {
    constexpr AnimationTask() { }
    constexpr AnimationTask(usize id, EasingTask<T>&& easingTask) : id(id), easingTask(std::move(easingTask)) { }

    constexpr EasingTask<T>& Easing() { return this->easingTask; }
    constexpr usize Id() const { return this->id; }
private:
    usize id;
    EasingTask<T> easingTask;
};

class GPXRenderer final {
public:
    GPXRenderer() : m() { }

    void Update(f32 dt, const GraphView& view, const usize selectedId, const SelectionType selectionType);
    void Render();

    void ReconstructView(const GraphView& view);
    void ReconstructEdges(const std::vector<gpx::Edge>& edges);

    void AddVertex(const Vertex& vtx);
    void AddEdge(const gpx::Edge& edge);

    void UpdateVertexPosition(usize id, f32x2 position);
    void UpdateVertexColor(usize id, u8x4 color);
    void UpdateEdgeColor(usize id, u8x4 color);
    void UpdateWeights();

    void EraseVertex(usize id);
    void EraseEdge(usize id);

    void SetCameraZoom(f32 zoom);
    f32 GetCameraZoom() const { return m.cameraZoom; }

    f32x2 GetCameraPosition() const { return m.cameraPosition; }
    void SetCameraPosition(f32x2 position);

    void SetViewport(u32x2 viewport);
    f32x2 ScreenToWorld(f32x2 screenPosition) const;

    static std::optional<GPXRenderer> Create(u32x2 initialViewport);
private:
    void UpdateRendererData();
    void UpdateAnimations(f32 dt);

    struct M {
        u32x2 viewport;
        f32x2 cameraPosition;
        f32 cameraZoom;

        GPXShaderData graphShaderData;
        StaticTextureBatch<BatchedVtxDimensions, ShaderVertex, IMG_VtxBatchDataTex, SMP_BatchDataSmp> batchedVertices; 
        StaticTextureBatch<BatchedEdgeDimensions, ShaderEdge, IMG_EdgeBatchDataTex, SMP_BatchDataSmp> batchedEdges; 

        std::vector<AnimationTask<f32>> vertexSizeAnimations;
        std::vector<AnimationTask<f32>> edgeSizeAnimations;

        GPXFontRenderer fontRenderer;
    } m;

    GPXRenderer(M&& m) : m(std::move(m)) { 
        this->UpdateRendererData();
    }
};
#endif
