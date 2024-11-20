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

struct BatchedChrData {
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

class GPXFontRenderer {
public:
    void Init(Graphexia_GlobalFontData_t* global);

    void DrawText(f32x2 position, f32 fontSize, std::string_view text);
    void DrawCharacter(f32x2 position, f32 fontSize, char32_t chr);
    void Clear();

    void Update();
    void Render();

    GlyphData& GetGlyphData(char32_t chr);
private:
    void LoadFont(std::string_view fontName);
    BMFont font;
    std::unordered_map<u32, GlyphData> glyphs;
    Graphexia_GlobalFontData_t* globalData;

    sg_sampler fontSampler;
    sg_image fontAtlas;
    sg_bindings fontBindings;
    StaticTextureBatch<BatchedTextureChrDimensions, BatchedChrData, IMG_ChrBatchDataTex, SMP_BatchDataSmp> batchedCharacters;

    BatchedChrData chrsData[(BatchedTextureChrDimensions * BatchedTextureChrDimensions) / 2];
};

#endif
