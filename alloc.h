#ifndef _alloc_H
#define _alloc_H

#include "types.h"

typedef enum {
    ALLOCATE,
    REALLOCATE,
    FREE,
} allocMode;

typedef enum {
    ALLOC_OK,
    ALLOC_FAIL,
    ALLOC_UNAVAILABLE,
} allocRes;

typedef allocRes (*allocator)(allocMode mode, void** mem, u64 size);

#ifdef STDLIB_ALLOC_IMPLEMENTATION
    #include <stdio.h>
    
    allocRes sysAlloc(allocMode mode, void** mem, u64 size) {
        switch (mode) {
        case ALLOCATE: {
            if (!mem) return ALLOC_FAIL;
            *mem = malloc(size);
            if (!(*mem)) return ALLOC_FAIL;
            return ALLOC_OK;
        } break;
        case REALLOCATE: {
            if (!mem) return ALLOC_FAIL;
            *mem = realloc(*mem, size);
            if (!(*mem)) return ALLOC_FAIL;
            return ALLOC_OK;
        } break;
        case FREE: {
            if (!mem) return ALLOC_FAIL;
            free(*mem);
            return ALLOC_OK;
        } break;
        }
        return ALLOC_UNAVAILABLE;
    }
#endif

#endif