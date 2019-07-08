/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GTypes_DEFINED
#define GTypes_DEFINED

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>

static inline void g_crash() {
    fprintf(stderr, "g_crash called\n");
    *(int*)0x50FF8001 = 12345;
}

#ifdef NDEBUG
    #define GRELEASE
    #define GASSERT(pred)
    #define GDEBUGCODE(code)
#else
    #define GDEBUG
    #define GASSERT(pred)       do { if (!(pred)) g_crash(); } while (0)
    #define GDEBUGCODE(code)    code
#endif

/**
 *  Given an array (not a pointer), this macro will return the number of
 *  elements declared in that array.
 */
#define GARRAY_COUNT(array) (int)(sizeof(array) / sizeof(array[0]))

bool is_dir(const char path[]);
bool mk_dir(const char path[]);

#endif
