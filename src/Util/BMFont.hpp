#ifndef _GRAPHEXIA_APP_UTIL_BMFONT__HPP_
#define _GRAPHEXIA_APP_UTIL_BMFONT__HPP_

#include <Graphexia/Core.hpp>
#include <unordered_map>
#include <string>
#include <sstream>
#include <vector>


struct BMInfo {
    std::string name;
    i16 fontSize;
    u8 bitfield, charSet;
    u16 stretchH;
    u8 antialiased;
    u8 paddingUp, paddingRight, paddingDown, paddingLeft;
    u8 spacingHorizontal, spacingVertical;
    u8 outlineThickness;
};

struct BMCommon {
    u16 lineHeight, base;
    u16 scaleW, scaleH;
    u8 bitfield;
    u8 a, r, g, b;
};

struct BMPage {
    std::string file;
};

struct BMChar {
    u16 x, y;
    u16 width, height;
    i16 xOffset, yOffset;
    i16 xAdvance;
    u8 page, channel;
};

struct BMKerning {
    i16 amount;
};

class BMFont final {
    constexpr BMFont(BMInfo&& info, BMCommon&& common, std::vector<BMPage>&& pages, std::unordered_map<u32, BMChar>&& chars, std::unordered_map<std::pair<u32, u32>, BMKerning>&& kerning); 


    static std::optional<BMFont> LoadFromStream(std::stringstream stream);
private:
    BMInfo info;   
    BMCommon common;

    std::vector<BMPage> pages;
    std::unordered_map<u32, BMChar> chars;
    std::unordered_map<std::pair<u32, u32>, BMKerning> kerning;
};

#endif
