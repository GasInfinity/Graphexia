#ifndef _GRAPHEXIA_APP_GPXFONTRENDERER__HPP_
#define _GRAPHEXIA_APP_GPXFONTRENDERER__HPP_

#include "Core.hpp"
#include "Render/StaticTextureBatch.hpp"
#include "Util/BMFont.hpp"

#include <sokol/sokol_gfx.h>
#include <string_view>
#include <unordered_map>

#include "BMFont.glsl.h"

const usize BatchedTextureChrDimensions = 1024;

struct ShaderChrData {
    f32x2 position;
    u32 unnormSizeHeight;
    u8x4 color;

    f16 u0; f16 v0;
    f16 u1; f16 v1;
    f16 u2; f16 v2;
    f16 u3; f16 v3;
};

struct GlyphData {
    f16 x1, y1;
    f16 x2, y2;
    u16 h;
    i8 xOffset, yOffset;
    u16 xAdvance;
};

struct GPXFontShaderData {
    // Model View Matrix
    f32 m00, m10, m20;
    f32 m01, m11, m22;
    f32 unused0, unused1;
};

class GPXFontRenderer {
public:
    GPXFontRenderer() : m() {}

    void DrawText(f32x2 position, f32 fontSize, std::string_view text);
    void DrawCharacter(f32x2 position, f32 fontSize, char chr);
    void Clear();

    void Update();
    void Render();

    GlyphData& GetGlyphData(char chr);

    void UpdateFontShaderData(const GPXFontShaderData& data);

    static std::optional<GPXFontRenderer> Create();
private:
    // TODO: Migrate to std::expected?
    static std::optional<std::pair<BMFont, sg_image>> LoadFont(std::string_view fontName);

    struct M {
        BMFont font;
        std::unordered_map<char32_t, GlyphData> glyphs;

        GPXFontShaderData fontShaderData;
        sg_sampler fontSampler;
        sg_image fontAtlas;
        sg_bindings fontBindings;
        StaticTextureBatch<BatchedTextureChrDimensions, ShaderChrData, IMG_ChrBatchDataTex, SMP_BatchDataSmp> batchedCharacters;
    } m;

    GPXFontRenderer(M&& m) : m(std::move(m)) { }
};

#endif
