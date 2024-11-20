#ifndef _GRAPHEXIA_APP_RENDER_STATICTEXTUREBATCH__HPP_
#define _GRAPHEXIA_APP_RENDER_STATICTEXTUREBATCH__HPP_

#include "Core.hpp"

#include <array>
#include <cstdlib>
#include <sokol/sokol_gfx.h>
#include <memory>
#include <optional>
#include <utility>

namespace detail {
    extern sg_sampler SharedNonFilteringSampler;
    extern usize ShaderNonFilteringSamplerRef;
} // namespace detail

// FIXME: Add destructor
template<usize TextureSize, typename BatchedData, usize BatchedImageBinding, usize BatchedSamplerBinding>
class StaticTextureBatch final {
private:
    constexpr static usize TotalByteSize = TextureSize * TextureSize * (sizeof(u32) * 4);
    constexpr static usize TotalBatchedSize = TotalByteSize / sizeof(BatchedData);


    struct M {
        sg_bindings batchBindings;
        sg_shader batchShader; sg_pipeline batchPipeline;
        sg_image batchedImage;

        std::unique_ptr<std::array<BatchedData, TotalBatchedSize>> batched;
        usize currentBatched;
        bool batchDirty;

        M(sg_bindings b, sg_shader s, sg_pipeline p, sg_image i, std::unique_ptr<std::array<BatchedData, TotalBatchedSize>> batched, usize c, bool d)
            : batchBindings(b), batchShader(s), batchPipeline(p), batchedImage(i), batched(std::move(batched)), currentBatched(c), batchDirty(d) {}

        M(M&&) = default;
        M& operator=(M&&) = default;
    } m;

    explicit StaticTextureBatch(M&& m) : m(std::move(m)) {}
public:
    explicit StaticTextureBatch() : m({{},{},{},{},{},0,false}) {}
    StaticTextureBatch(StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>& other) = delete;
    StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>& operator =(StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>& other) = delete;

    StaticTextureBatch(StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>&&) = default;
    StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>& operator =(StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>&&) noexcept = default;

    void Update();

    template<typename R>
    void Render(R beforeDrawCall);

    sg_image BatchedImage() { return m.batchedImage; }
    std::array<BatchedData, TotalBatchedSize>& Data() { return *m.batched.get(); }
    void FlagDirty() { m.batchDirty = true; }

    void SetBatchedCount(usize newBatched) { this->FlagDirty(); m.currentBatched = newBatched; }
    usize BatchedCount() const { return m.currentBatched; }

    bool IsEmpty() const { return BatchedCount() == 0; }
    void ClearBatched() { SetBatchedCount(0); }

    static std::optional<StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>> Create(const sg_shader_desc* shaderDesc, const sg_bindings& preBindings);
};

template<usize TextureSize, typename BatchedData, usize BatchedImageBinding, usize BatchedSamplerBinding>
void StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>::Update() {
    if(m.batchDirty) {
        sg_image_data batchImageData{};
        batchImageData.subimage[0][0] = sg_range{ .ptr = m.batched->data(), .size = TotalByteSize };
        sg_update_image(m.batchedImage, &batchImageData);
        m.batchDirty = false;
    }
}

template<usize TextureSize, typename BatchedData, usize BatchedImageBinding, usize BatchedSamplerBinding>
template<typename R>
void StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>::Render(R beforeDrawCall) {
    if(IsEmpty()){
        return;
    } 

    sg_apply_pipeline(m.batchPipeline);
    sg_apply_bindings(m.batchBindings);
    beforeDrawCall();
    sg_draw(0, m.currentBatched * 6, 1);
}

template<usize TextureSize, typename BatchedData, usize BatchedImageBinding, usize BatchedSamplerBinding>
std::optional<StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>> StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>::Create(const sg_shader_desc* shaderDesc, const sg_bindings& preBindings) {
    static_assert(TextureSize > 0 && (TextureSize & 1) == 0, "Texture size must be a valid power of two and not zero"); 

    // FIXME: Deallocate this
    if(detail::ShaderNonFilteringSamplerRef++ == 0) {
        sg_sampler_desc nonFilteringSamplerDesc{};
        nonFilteringSamplerDesc.label = "Shared Non Filtering Data Sampler";
        nonFilteringSamplerDesc.min_filter = SG_FILTER_NEAREST;
        nonFilteringSamplerDesc.mag_filter = SG_FILTER_NEAREST;
        nonFilteringSamplerDesc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        nonFilteringSamplerDesc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        detail::SharedNonFilteringSampler = sg_make_sampler(&nonFilteringSamplerDesc);
        
        if(sg_query_sampler_state(detail::SharedNonFilteringSampler) != SG_RESOURCESTATE_VALID) {
            std::abort();
        }
    }

    sg_image_desc batchedImageDesc{};
    batchedImageDesc.label = "Static Texture Batched Data";
    batchedImageDesc.width = TextureSize;
    batchedImageDesc.height = TextureSize;
    batchedImageDesc.pixel_format = SG_PIXELFORMAT_RGBA32UI;
    batchedImageDesc.usage = SG_USAGE_STREAM;
    batchedImageDesc.num_mipmaps = 1;
    sg_image batchedImage = sg_make_image(&batchedImageDesc);

    if(sg_query_image_state(batchedImage) != SG_RESOURCESTATE_VALID) {
        return std::nullopt;
    }

    sg_shader batchShader = sg_make_shader(shaderDesc);

    if(sg_query_shader_state(batchShader) != SG_RESOURCESTATE_VALID) {
        sg_destroy_image(batchedImage);
        return std::nullopt;
    }

    sg_pipeline_desc batchPipelineDesc{};
    batchPipelineDesc.label = "Graph Edges Pipeline";
    batchPipelineDesc.shader = batchShader; 
    batchPipelineDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    batchPipelineDesc.colors[0].blend = {
        .enabled = true,
        .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
        .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .op_rgb = SG_BLENDOP_ADD,
        .src_factor_alpha = SG_BLENDFACTOR_ONE,
        .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        .op_alpha = SG_BLENDOP_ADD
    };

    sg_pipeline batchPipeline = sg_make_pipeline(&batchPipelineDesc);
    
    if(sg_query_pipeline_state(batchPipeline) != SG_RESOURCESTATE_VALID) {
        sg_destroy_image(batchedImage);
        sg_destroy_shader(batchShader);
        return std::nullopt;
    }

    sg_bindings batchBindings = preBindings;
    batchBindings.images[BatchedImageBinding].id = batchedImage.id;
    batchBindings.samplers[BatchedSamplerBinding].id = detail::SharedNonFilteringSampler.id;

    return StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>(StaticTextureBatch<TextureSize, BatchedData, BatchedImageBinding, BatchedSamplerBinding>::M(
        batchBindings,
        batchShader,
        batchPipeline,
        batchedImage,
        std::make_unique<std::array<BatchedData, TotalBatchedSize>>(),
        0,
        false
    ));
}

#endif
