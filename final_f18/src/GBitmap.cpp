/**
 *  Copyright 2015 Mike Reed
 */

#include "GBitmap.h"
#include "lodepng.h"

void GBitmap::setIsOpaque(IsOpaque io) {
    switch (io) {
        case kYes_IsOpaque: fIsOpaque = true;  break;
        case  kNo_IsOpaque: fIsOpaque = false; break;
        case kCompute_IsOpaque: this->computeIsOpaque(); break;
    }
}

void GBitmap::reset(int w, int h, size_t rb, GPixel* pixels, IsOpaque io) {
    fWidth = w;
    fHeight = h;
    fRowBytes = rb;
    fPixels = pixels;
    this->setIsOpaque(io);
    this->validate();
}

bool GBitmap::ComputeIsOpaque(const GBitmap& bm) {
    for (int y = 0; y < bm.height(); ++y) {
        const GPixel* row = bm.getAddr(0, y);
        for (int x = 0; x < bm.width(); ++x) {
            if (GPixel_GetA(row[x]) != 0xFF) {
                return false;
            }
        }
    }
    return true;
}

void GBitmap::alloc(int w, int h, size_t rb) {
    GASSERT(w >= 0);
    GASSERT(h >= 0);
    if (rb == 0) {
        rb = w * sizeof(GPixel);
    }
    fWidth = w;
    fHeight = h;
    fRowBytes = rb;

    this->reset(w, h, rb,
                (w > 0 && h > 0) ? (GPixel*)calloc(h, rb) : nullptr,
                kNo_IsOpaque);
}

