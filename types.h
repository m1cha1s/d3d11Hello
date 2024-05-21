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

// Boolean

typedef bool u8;
#define true 1
#define false 0

// String

typedef struct {
    u64 len;
    u8* data;
} string;

#ifdef TYPES_IMPLEMENTATION
    #include <string.h>
    #include "alloc.h"
    
    str cstrString(allocator alloc, u8* cstr) {
        u64 len = strlen(cstr);
        
        string str = {0};
        
        if (alloc(ALLOCATE, &str.data, len)) {
            assert("We failed to allocate!!!" && false);
            return str;
        }
        
        str.len = len;
        return str;        
    }
    
    void freeString(allocator alloc, string str) {
        if (!str.data) return;
        alloc(FREE, &str.data, 0);
    }
    
    u8* stringCStr(allocator alloc, string str) {
        if (!str.data) return;
        
        u8* data = NULL;
        if (alloc(ALLOCATE, &data, str.len+1)) {
            assert("We failed to allocate!!!" && false);
            return NULL;
        }
        
        for (s64 i = 0; i < str.len; i++) {
            data[i] = str.data[i];
        }
        data[str.len] = '\0'; // Null terminator.
        
        return data;
    }
#endif

// GFX specific

// This may change.
typedef u64 ShaderIdx;

#endif