#ifndef _GRAPHEXIA_CORE__HPP_
#define _GRAPHEXIA_CORE__HPP_

#include <cstdint>
#include <cstddef>

typedef uint8_t u8;
typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef size_t usize;

typedef float f32;


namespace gpx {
    struct Point {
        i16 x, y;
    };
}

#endif
