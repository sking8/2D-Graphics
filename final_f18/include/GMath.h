/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GMath_DEFINED
#define GMath_DEFINED

#include "GTypes.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline int GRoundToInt(float x) {
    return (int)floorf(x + 0.5f);
}

static inline int GFloorToInt(float x) {
    return (int)floorf(x);
}

static inline int GCeilToInt(float x) {
    return (int)ceilf(x);
}

static inline float GPinToUnit(float x) {
    return std::max(0.0f, std::min(1.0f, x));
}

#endif
