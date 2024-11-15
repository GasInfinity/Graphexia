#include "GPXFontRenderer.hpp"

#include <cstdint>
#include <stb/stb_image.h>

#include <cassert>
#include <fstream>
#include <string>

// FIXME: Refactor!
void GPXFontRenderer::Init(Graphexia_GlobalFontData_t* globalData) {
    this->LoadFont();
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

    f32 scale = std::min(fontSize / 82.0, 1.0);
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

void GPXFontRenderer::DrawCharacter(f32x2 position, f32 fontSize, char32_t chr) {
    GlyphData& glyph = this->GetGlyphData(chr);
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

void GPXFontRenderer::LoadFont() {
    std::ifstream defFile("./assets/fonts/sans-serif/def.fnt");

    std::string line;
    std::getline(defFile, line);
    std::getline(defFile, line);
    std::getline(defFile, line);
    std::getline(defFile, line);

    const int Size = 512;

    usize charsCount = std::strtoull((line.begin() + line.find('=') + 1).base(), nullptr, 10);
    this->glyphs.reserve(charsCount);

    for(usize i = 0; i < charsCount; ++i) {
        if(!std::getline(defFile, line)) {
            assert(false);
        }

        char32_t id = std::strtol((line.begin() + line.find("id") + 3).base(), nullptr, 10);
        usize x = std::strtol((line.begin() + line.find('x') + 2).base(), nullptr, 10);
        usize y = std::strtol((line.begin() + line.find('y') + 2).base(), nullptr, 10);
        usize width = std::strtol((line.begin() + line.find("width") + 6).base(), nullptr, 10);
        usize height = std::strtol((line.begin() + line.find("height") + 7).base(), nullptr, 10);
        i16 xoffset= std::strtol((line.begin() + line.find("xoffset") + 8).base(), nullptr, 10);
        i16 yoffset = std::strtol((line.begin() + line.find("yoffset") + 8).base(), nullptr, 10);
        usize xadvance = std::strtol((line.begin() + line.find("xadvance") + 9).base(), nullptr, 10);
        // We can skip page and channel for now

        // std::cout << static_cast<u32>(id) << "|: " << xoffset << ", " << yoffset << " || " << xadvance << std::endl;
        this->glyphs[id] = {
            static_cast<f16>(x / static_cast<f32>(Size)), static_cast<f16>(y / static_cast<f32>(Size)),
            static_cast<f16>((x + width) / static_cast<f32>(Size)), static_cast<f16>((y + height) / static_cast<f32>(Size)),
            static_cast<u16>(width), static_cast<u16>(height),
            static_cast<i8>(xoffset), static_cast<i8>(yoffset),
            static_cast<u16>(xadvance)
        };
    }

    FILE* atlasImage = fopen("./assets/fonts/sans-serif/base.png", "r");

    int w, h, c;
    stbi_uc* data = stbi_load_from_file(atlasImage, &w, &h, &c, 4);

    sg_image_desc fontAtlasDesc{};
    fontAtlasDesc.label = "Font Atlas";
    fontAtlasDesc.width = Size;
    fontAtlasDesc.height = Size;
    fontAtlasDesc.pixel_format = SG_PIXELFORMAT_RGBA8;
    fontAtlasDesc.usage = SG_USAGE_IMMUTABLE;
    fontAtlasDesc.num_mipmaps = 1;
    fontAtlasDesc.data.subimage[0][0] = { .ptr = data, .size = Size * Size * 4 };
    this->fontAtlas = sg_make_image(&fontAtlasDesc);

    stbi_image_free(data);
}
