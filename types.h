#ifndef _types_H
#define _types_H

#include <stdint.h>

// Numbers

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

// Some math stuff

struct v2 {
    f32 x;
    f32 y;
};

struct v3 {
    f32 x;
    f32 y;
    f32 z;
};

struct v4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
};

// GFX specific

// This may change.
typedef u64 ShaderIdx;

typedef enum {
    VertexShader,
    PixelShader,
} ShaderType;

#endif