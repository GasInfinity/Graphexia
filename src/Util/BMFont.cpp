#include "BMFont.hpp"
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

BMFont::BMFont(BMInfo&& info, BMCommon&& common, std::vector<BMPage>&& pages, std::unordered_map<u32, BMChar>&& chars, std::unordered_map<u64, BMKerning>&& kerning)
    : info(std::move(info)), common(std::move(common)), pages(std::move(pages)), chars(std::move(chars)), kerning(std::move(kerning)) {}

// FIXME: Use custom parsing or parsing library, don't rely on sscanf...
BMInfo ParseInfo(std::string_view attributes) {
    char face[256];
    i32 fontSize;
    char bold; char italic;
    char unicode;
    u32 stretchH;
    char aa;
    char smooth;
    u32 paddingUp, paddingRight, paddingDown, paddingLeft;
    u32 spacingHorizontal, spacingVertical;
    u32 outlineThickness;
    std::sscanf(attributes.data(), "info face=\"%255[^\"]\" size=%d bold=%c italic=%c charset=\"\" unicode=%c stretchH=%u smooth=%c aa=%c padding=%u,%u,%u,%u spacing=%u,%u outline=%u",
                face, &fontSize, &bold, &italic, &unicode, &stretchH, &smooth, &aa, &paddingUp, &paddingRight, &paddingDown, &paddingLeft, &spacingHorizontal, &spacingVertical, &outlineThickness);
    BMInfoFlags flags = BMInfoFlags::None;

    if(bold) flags = flags | BMInfoFlags::Bold;
    if(italic) flags = flags | BMInfoFlags::Italic;
    if(unicode) flags = flags | BMInfoFlags::Unicode;
    if(smooth) flags = flags | BMInfoFlags::Smooth;

    return BMInfo{
        static_cast<i16>(fontSize),
        flags,
        0,
        static_cast<u16>(stretchH),
        aa != 0,
        static_cast<u8>(paddingUp), static_cast<u8>(paddingRight), static_cast<u8>(paddingDown), static_cast<u8>(paddingLeft),
        static_cast<u8>(spacingHorizontal), static_cast<u8>(spacingVertical),
        static_cast<u8>(outlineThickness),
        std::string(face)
    };
}

BMCommon ParseCommon(std::string_view attributes) {
    u32 lineHeight, base;
    u32 scaleW, scaleH;
    u32 pages;
    char packed;

    std::sscanf(attributes.data(), "common lineHeight=%u base=%u scaleW=%u scaleH=%u pages=%u packed=%c",
                &lineHeight, &base, &scaleW, &scaleH, &pages, &packed);
    return {
        static_cast<u16>(lineHeight), static_cast<u16>(base),
        static_cast<u16>(scaleW), static_cast<u16>(scaleH),
        static_cast<u16>(pages),
        packed ? BMCommonFlags::Packed : BMCommonFlags::None, 
        0, 0, 0, 0
    };
}

BMFont BMFont::LoadFromStream(std::istream& stream) {
    std::string line;
    std::getline(stream, line);
    BMInfo info = ParseInfo(line);
    std::getline(stream, line);
    BMCommon common = ParseCommon(line);
    
    std::vector<BMPage> pages(common.pages);

    char buf[256];
    for (usize i = 0; i < common.pages; ++i) {
        std::getline(stream, line);
        u32 id;
        std::sscanf(line.data(), "page id=%u file=\"%255[^\"]\"", &id, buf);
        
        pages[id] = BMPage{
            std::string(buf)
        };
    }

    std::getline(stream, line);
    u32 charsCount;
    std::sscanf(line.c_str(), "chars count=%u", &charsCount);

    std::unordered_map<u32, BMChar> chars;
    chars.reserve(charsCount);
    for (usize i = 0; i < charsCount; ++i) {
        std::getline(stream, line);

        u32 id;
        u32 x, y;
        u32 w, h;
        i32 xOffset, yOffset;
        u32 xAdvance;
        u32 page;
        u32 channel;
        std::sscanf(line.data(), "char id=%u x=%u y=%u width=%u height=%u xoffset=%d yoffset=%d xadvance=%u page=%u chnl=%u",
                    &id, &x, &y, &w, &h, &xOffset, &yOffset, &xAdvance, &page, &channel);
        
        chars[id] = BMChar{
            static_cast<u16>(x), static_cast<u16>(y),
            static_cast<u16>(w), static_cast<u16>(h),
            static_cast<i16>(xOffset), static_cast<i16>(yOffset),
            static_cast<i16>(xAdvance),
            static_cast<u8>(page), static_cast<u8>(channel)
        };
    }

    // TODO: Kerning
    std::unordered_map<u64, BMKerning> kerning;
    if(std::getline(stream, line)) {
    }

    return BMFont(std::move(info), std::move(common), std::move(pages), std::move(chars), std::move(kerning));
}
