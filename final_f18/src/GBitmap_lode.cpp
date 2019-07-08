/**
 *  Copyright 2018 Mike Reed
 */

#include "GBitmap.h"
#include "lodepng.h"

static void convertToPNG(const GPixel src[], int width, uint8_t dst[]) {
    for (int i = 0; i < width; i++) {
        GPixel c = *src++;
        int a = GPixel_GetA(c);
        int r = GPixel_GetR(c);
        int g = GPixel_GetG(c);
        int b = GPixel_GetB(c);
        
        // PNG requires unpremultiplied, but GPixel is premultiplied
        if (0 != a && 255 != a) {
            r = (r * 255 + a/2) / a;
            g = (g * 255 + a/2) / a;
            b = (b * 255 + a/2) / a;
        }
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
        *dst++ = a;
    }
}

bool GBitmap::writeToFile(const char path[]) const {
    size_t rb = this->width() * 4;
    uint8_t* pix = (uint8_t*)malloc(this->height() * rb);
    if (!pix) {
        return false;
    }

    const GPixel* src = this->pixels();
    uint8_t* dst = pix;
    for (int y = 0; y < this->height(); ++y) {
        convertToPNG(src, this->width(), dst);
        src += this->rowBytes() / 4;
        dst += rb;
    }

    unsigned err = lodepng_encode32_file(path, pix, this->width(), this->height());
    free(pix);
    return err == 0;
}

///////////////////////////////////////////////////////////////////////////////

static int alpha_mul(unsigned a, unsigned c) {
    return (a * c + 127) / 255;
}

static void swizzle_rgba_row(GPixel dst[], const uint8_t src[], int count) {
    for (int i = 0; i < count; ++i) {
        unsigned a = src[3];
        dst[i] = GPixel_PackARGB(a,
                                 alpha_mul(a, src[0]),
                                 alpha_mul(a, src[1]),
                                 alpha_mul(a, src[2]));
        src += 4;
    }
}

bool GBitmap::readFromFile(const char path[]) {
    unsigned w, h;
    unsigned char* pix = nullptr;
    if (lodepng_decode32_file(&pix, &w, &h, path)) {
        free(pix);
        return false;
    }

    this->alloc(w, h);

    GPixel* dst = this->pixels();
    const uint8_t* src = pix;
    size_t rb = w * 4;
    for (unsigned y = 0; y < h; ++y) {
        swizzle_rgba_row(dst, src, w);
        src += rb;
        dst += this->rowBytes() / 4;
    }
    free(pix);
    return true;
}


