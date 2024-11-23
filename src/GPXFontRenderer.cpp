#include "GPXFontRenderer.hpp"
#include "Render/StaticTextureBatch.hpp"
#include "Util/BMFont.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stb/stb_image.h>

#include <cassert>
#include <fstream>
#include <string_view>
#include <utility>

void GPXFontRenderer::DrawText(f32x2 position, f32 fontSize, std::string_view text) {
    f32x2 currentPosition = position;

    f32 scale = std::min(fontSize / static_cast<f32>(m.font.Info().fontSize), 1.f);
    u16 unormScale = scale * UINT16_MAX;

    auto& batchedCharactersData = m.batchedCharacters.Data();
    usize currentBatched = m.batchedCharacters.BatchedCount();
    // TODO: Unicode? Not today at least...
    for (char chr : text) {
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

    m.batchedCharacters.SetBatchedCount(currentBatched);
}

// Ironically, it's less code duplication and achieves almost the same performance...
// Just use DrawText
void GPXFontRenderer::DrawCharacter(f32x2 position, f32 fontSize, char chr) {
    DrawText(position, fontSize, std::string_view(&chr, 1));
}

void GPXFontRenderer::Clear() {
    m.batchedCharacters.ClearBatched(); 
}

void GPXFontRenderer::Update() {
    m.batchedCharacters.Update();
}

void GPXFontRenderer::Render() {
    auto globals = m.fontShaderData;
    m.batchedCharacters.Render([&globals]() {
        sg_apply_uniforms(UB_GlobalFontData, SG_RANGE(globals));
    });
}

GlyphData& GPXFontRenderer::GetGlyphData(char chr) {
    auto foundGlyph = m.glyphs.find(chr);
    return foundGlyph != m.glyphs.end() ? foundGlyph->second : m.glyphs.at(U'�');
}

void GPXFontRenderer::UpdateFontShaderData(const GPXFontShaderData& fontShaderData) {
    m.fontShaderData= fontShaderData;
}

std::optional<std::pair<BMFont, sg_image>> GPXFontRenderer::LoadFont(std::string_view fontName) {
    std::filesystem::path fontPath = "./assets/fonts/";
    fontPath /= fontName;
    std::ifstream defFile((fontPath / "def.fnt"));
    const BMFont font = BMFont::LoadFromStream(defFile);

    const BMPage& first = font.Pages()[0];
    std::clog << "Loading font atlas for " << font.Info().name << " at: " << (fontPath / first.file) << std::endl;
    FILE* atlasImage = fopen((fontPath / first.file).c_str(), "rb");

    if(!atlasImage) {
        std::cerr << "Error opening atlas image" << std::endl;
        return std::nullopt;
    }

    int w, h, c;
    stbi_uc* data = stbi_load_from_file(atlasImage, &w, &h, &c, 4);

    if(!data) {
        std::cerr << "Error parsing png" << std::endl;
        std::cerr << stbi_failure_reason() << std::endl;
        return std::nullopt;
    }

    const BMCommon& common = font.Common();

    sg_image_desc fontAtlasDesc{};
    fontAtlasDesc.label = "Font Atlas";
    fontAtlasDesc.width = common.scaleW;
    fontAtlasDesc.height = common.scaleH;
    fontAtlasDesc.pixel_format = SG_PIXELFORMAT_RGBA8;
    fontAtlasDesc.usage = SG_USAGE_IMMUTABLE;
    fontAtlasDesc.num_mipmaps = 1;
    fontAtlasDesc.data.subimage[0][0] = { .ptr = data, .size = static_cast<usize>(common.scaleW) * common.scaleH * 4 };
    sg_image fontAtlas = sg_make_image(&fontAtlasDesc);

    stbi_image_free(data);
    fclose(atlasImage);

    if(sg_query_image_state(fontAtlas) != SG_RESOURCESTATE_VALID) {
        return std::nullopt;
    }

    return std::make_pair(font, fontAtlas);
}

std::optional<GPXFontRenderer> GPXFontRenderer::Create() {
    std::optional<std::pair<BMFont, sg_image>> loadedFont = GPXFontRenderer::LoadFont("sans-serif");

    if(!loadedFont) {
        return std::nullopt;
    }

    BMFont& font = loadedFont->first;
    sg_image fontAtlas = loadedFont->second;

    const BMCommon& common = font.Common();
    const std::unordered_map<u32, BMChar>& chars = font.Chars();
    std::unordered_map<char32_t, GlyphData> glyphs;
    glyphs.reserve(chars.size());

    for (auto [id, ch] : chars) {
        glyphs[id] = {
            static_cast<f16>(ch.x / static_cast<f32>(common.scaleW)), static_cast<f16>(ch.y / static_cast<f32>(common.scaleH)),
            static_cast<f16>((ch.x + ch.width) / static_cast<f32>(common.scaleW)), static_cast<f16>((ch.y + ch.height) / static_cast<f32>(common.scaleH)),
            ch.height,
            static_cast<i8>(ch.xOffset), static_cast<i8>(ch.yOffset),
            static_cast<u16>(ch.xAdvance)
        }; 
    }

    sg_sampler_desc fontSamplerDesc{};
    fontSamplerDesc.label = "Font Sampler";
    fontSamplerDesc.min_filter = SG_FILTER_LINEAR;
    fontSamplerDesc.mag_filter = SG_FILTER_LINEAR;
    fontSamplerDesc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    fontSamplerDesc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    sg_sampler fontSampler = sg_make_sampler(&fontSamplerDesc);

    if(sg_query_sampler_state(fontSampler) != SG_RESOURCESTATE_VALID) {
        return std::nullopt;
    }

    sg_bindings fontBindings{};
    fontBindings.images[IMG_FontTex].id = fontAtlas.id;
    fontBindings.samplers[SMP_TexSmp].id = fontSampler.id;

    auto batchedCharacters = StaticTextureBatch<BatchedTextureChrDimensions, ShaderChrData, IMG_ChrBatchDataTex, SMP_BatchDataSmp>::Create(
        Graphexia_BMFont_shader_desc(sg_query_backend()), fontBindings 
    );

    if(!batchedCharacters) {
        sg_destroy_sampler(fontSampler);
        return std::nullopt;
    }

    return GPXFontRenderer(M{
        font,
        std::move(glyphs),
        {},
        fontSampler,
        fontAtlas,
        fontBindings,
        std::move(*batchedCharacters),
    });
}

