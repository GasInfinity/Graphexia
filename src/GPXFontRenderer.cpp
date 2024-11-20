#include "GPXFontRenderer.hpp"
#include "Util/BMFont.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stb/stb_image.h>

#include <cassert>
#include <fstream>

// FIXME: Refactor!
void GPXFontRenderer::Init(Graphexia_GlobalFontData_t* globalData) {
    this->LoadFont("sans-serif");
    this->globalData = globalData;

    sg_sampler_desc fontSamplerDesc{};
    fontSamplerDesc.label = "Font Sampler";
    fontSamplerDesc.min_filter = SG_FILTER_LINEAR;
    fontSamplerDesc.mag_filter = SG_FILTER_LINEAR;
    fontSamplerDesc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    fontSamplerDesc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    this->fontSampler = sg_make_sampler(&fontSamplerDesc);

    sg_bindings fontBindings{};
    fontBindings.images[IMG_FontTex].id = this->fontAtlas.id;
    fontBindings.samplers[SMP_TexSmp].id = this->fontSampler.id;

    this->batchedCharacters = StaticTextureBatch<BatchedTextureChrDimensions, BatchedChrData, IMG_ChrBatchDataTex, SMP_BatchDataSmp>::Create(
        Graphexia_BMFont_shader_desc(sg_query_backend()), fontBindings 
    ).value();
}

void GPXFontRenderer::DrawText(f32x2 position, f32 fontSize, std::string_view text) {
    f32x2 currentPosition = position;

    f32 scale = std::min(fontSize / static_cast<f32>(this->font.Info().fontSize), 1.f);
    u16 unormScale = scale * UINT16_MAX;

    auto& batchedCharactersData = this->batchedCharacters.Data();
    usize currentBatched = this->batchedCharacters.BatchedCount();
    for (char32_t chr : text) {
        GlyphData& glyph = this->GetGlyphData(chr);

        u32 unormSizeHeight = unormScale << 16 | glyph.h;
        batchedCharactersData.at(currentBatched++) = {
            {currentPosition.x + glyph.xOffset * scale, currentPosition.y + glyph.yOffset * scale}, unormSizeHeight, Rgba8(0xFFFFFFFF),
            glyph.x1, glyph.y1,
            glyph.x2, glyph.y1,
            glyph.x1, glyph.y2,
            glyph.x2, glyph.y2
        };

        currentPosition.x += glyph.xAdvance * scale;
    }

    this->batchedCharacters.SetBatchedCount(currentBatched);
}

// TODO
void GPXFontRenderer::DrawCharacter(f32x2 position, f32 fontSize, char32_t chr) {
    GlyphData& glyph = this->GetGlyphData(chr);
}

void GPXFontRenderer::Clear() {
    this->batchedCharacters.ClearBatched(); 
}

void GPXFontRenderer::Update() {
    this->batchedCharacters.Update();
}

void GPXFontRenderer::Render() {
    auto globals = this->globalData;
    this->batchedCharacters.Render([&globals]() {
        sg_apply_uniforms(UB_GlobalFontData, SG_RANGE(*globals));
    });
}

GlyphData& GPXFontRenderer::GetGlyphData(char32_t chr) {
    auto foundGlyph = this->glyphs.find(chr);
    return foundGlyph != this->glyphs.end() ? foundGlyph->second : this->glyphs.at(U'�');
}

void GPXFontRenderer::LoadFont(std::string_view fontName) {
    std::filesystem::path fontPath = "./assets/fonts/";
    fontPath /= fontName;
    std::ifstream defFile((fontPath / "def.fnt"));
    this->font = BMFont::LoadFromStream(defFile);

    const BMCommon& common = this->font.Common();
    const std::unordered_map<u32, BMChar>& chars = this->font.Chars();
    this->glyphs.reserve(chars.size());

    for (auto [id, ch] : chars) {
        this->glyphs[id] = {
            static_cast<f16>(ch.x / static_cast<f32>(common.scaleW)), static_cast<f16>(ch.y / static_cast<f32>(common.scaleH)),
            static_cast<f16>((ch.x + ch.width) / static_cast<f32>(common.scaleW)), static_cast<f16>((ch.y + ch.height) / static_cast<f32>(common.scaleH)),
            ch.height,
            static_cast<i8>(ch.xOffset), static_cast<i8>(ch.yOffset),
            static_cast<u16>(ch.xAdvance)
        }; 
    }
    
    const BMPage& first = this->font.Pages()[0];
    std::clog << "Loading font atlas for " << this->font.Info().name << " at: " << (fontPath / first.file) << std::endl;
    FILE* atlasImage = fopen((fontPath / first.file).c_str(), "rb");

    if(!atlasImage) {
        std::cerr << "Error opening atlas image" << std::endl;
        std::abort();
    }

    int w, h, c;
    stbi_uc* data = stbi_load_from_file(atlasImage, &w, &h, &c, 4);

    if(!data) {
        std::cerr << "Error parsing png" << std::endl;
        std::cerr << stbi_failure_reason() << std::endl;
        std::abort();
    }

    sg_image_desc fontAtlasDesc{};
    fontAtlasDesc.label = "Font Atlas";
    fontAtlasDesc.width = common.scaleW;
    fontAtlasDesc.height = common.scaleH;
    fontAtlasDesc.pixel_format = SG_PIXELFORMAT_RGBA8;
    fontAtlasDesc.usage = SG_USAGE_IMMUTABLE;
    fontAtlasDesc.num_mipmaps = 1;
    fontAtlasDesc.data.subimage[0][0] = { .ptr = data, .size = static_cast<usize>(common.scaleW) * common.scaleH * 4 };
    this->fontAtlas = sg_make_image(&fontAtlasDesc);

    stbi_image_free(data);
    fclose(atlasImage);
}
