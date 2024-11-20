#ifndef _GRAPHEXIA_APP_UTIL_BMFONT__HPP_
#define _GRAPHEXIA_APP_UTIL_BMFONT__HPP_

#include <Graphexia/Core.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <istream>


enum class BMInfoFlags : u8 {
    None = 0,
    Smooth = 1 << 0,
    Unicode = 1 << 1,
    Italic = 1 << 2,
    Bold = 1 << 3,
    FixedHeight = 1 << 4,
};

inline BMInfoFlags operator|(BMInfoFlags a, BMInfoFlags b) { return static_cast<BMInfoFlags>(static_cast<u8>(a) | static_cast<u8>(b)); }
inline BMInfoFlags operator&(BMInfoFlags a, BMInfoFlags b) { return static_cast<BMInfoFlags>(static_cast<u8>(a) & static_cast<u8>(b)); }

struct BMInfo {
    i16 fontSize;
    BMInfoFlags flags;
    u8 charSet;
    u16 stretchH;
    bool antialiased;
    u8 paddingUp, paddingRight, paddingDown, paddingLeft;
    u8 spacingHorizontal, spacingVertical;
    u8 outlineThickness;
    std::string name;
};

enum class BMCommonFlags : u8 {
    None = 0,
    Packed = 1 << 7
};

inline BMCommonFlags operator|(BMCommonFlags a, BMCommonFlags b) { return static_cast<BMCommonFlags>(static_cast<u8>(a) | static_cast<u8>(b)); }
inline BMCommonFlags operator&(BMCommonFlags a, BMCommonFlags b) { return static_cast<BMCommonFlags>(static_cast<u8>(a) & static_cast<u8>(b)); }

struct BMCommon {
    u16 lineHeight, base;
    u16 scaleW, scaleH;
    u16 pages;
    BMCommonFlags flags;
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
public:
    explicit BMFont() = default;
    explicit BMFont(BMInfo&& info, BMCommon&& common, std::vector<BMPage>&& pages, std::unordered_map<u32, BMChar>&& chars, std::unordered_map<u64, BMKerning>&& kerning); 

    const BMInfo& Info() const { return this->info; }
    const BMCommon& Common() const { return this->common; }

    const std::vector<BMPage>& Pages() const { return this->pages; }
    const std::unordered_map<u32, BMChar>& Chars() const { return this->chars; }

    static BMFont LoadFromStream(std::istream& stream);
private:
    BMInfo info;   
    BMCommon common;

    std::vector<BMPage> pages;
    std::unordered_map<u32, BMChar> chars;
    std::unordered_map<u64, BMKerning> kerning;
};

#endif
