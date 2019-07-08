/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GBitmap_DEFINED
#define GBitmap_DEFINED

#include "GPixel.h"

class GBitmap {
public:
    GBitmap() { this->reset(); }

    GBitmap(int w, int h, size_t rb, GPixel* pixels, bool isOpaque)
        : fWidth(w), fHeight(h), fRowBytes(rb), fPixels(pixels), fIsOpaque(isOpaque)
    {
        this->validate();
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    size_t rowBytes() const { return fRowBytes; }
    GPixel* pixels() const { return fPixels; }
    bool isOpaque() const { return fIsOpaque; }

    void reset() {
        fWidth = 0;
        fHeight = 0;
        fPixels = NULL;
        fRowBytes = 0;
        fIsOpaque = false;  // unknown
    }

    enum IsOpaque {
        kNo_IsOpaque,
        kYes_IsOpaque,
        kCompute_IsOpaque,
    };
    void reset(int w, int h, size_t rb, GPixel* pixels, IsOpaque);

    GPixel* getAddr(int x, int y) const {
        GASSERT(x >= 0 && x < this->width());
        GASSERT(y >= 0 && y < this->height());
        return this->pixels() + x + (y * this->rowBytes() >> 2);
    }

    void setIsOpaque(IsOpaque);

    /**
     *  Inspect the bitmap's pixels to determine if all the alpha values are 0xFF. This sets the
     *  bitmap's isAlpha attrbute to the result.
     */
    void computeIsOpaque() {
        fIsOpaque = ComputeIsOpaque(*this);
    }

    /**
     *  Attempt to read the png image stored in the named file.
     *
     *  On success, allocate the memory for the pixels using malloc() and set bitmap to the result,
     *  returning true. The caller must call free(bitmap->fPixels) when they are finished.
     *
     *  This automatically computes the opaqueness of the bitmap.
     *
     *  On failure, return false and bitmap is reset to empty.
     */
    bool readFromFile(const char path[]);

    /*
     *  Attempt to write the bitmap as a PNG into a new file (the file will be created/overwritten).
     *  Return true on success.
     */
    bool writeToFile(const char path[]) const;

    /**
     *  Allocate the memory for the bitmap. If rowBytes is 0, it will be computed from w.
     */
    void alloc(int w, int h, size_t rowBytes = 0);

private:
    int     fWidth;
    int     fHeight;
    GPixel* fPixels;
    size_t  fRowBytes;
    bool    fIsOpaque;  // hint that all pixels have 0xFF for alpha

    void validate() const {
        GASSERT(fWidth >= 0);
        GASSERT(fHeight >= 0);
        GASSERT((unsigned)fWidth <= fRowBytes >> 2);

        if (fIsOpaque == kYes_IsOpaque) {
            GASSERT(ComputeIsOpaque(*this));
        }
    }

    static bool ComputeIsOpaque(const GBitmap&);
};

template <typename S> void visit_pixels(const GBitmap& bm, S&& visitor) {
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            visitor(x, y, bm.getAddr(x, y));
        }
    }
}

#endif
