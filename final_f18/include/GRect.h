/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GRect_DEFINED
#define GRect_DEFINED

#include "GMath.h"

template <typename T> class GTRect {
public:
    T   fLeft;
    T   fTop;
    T   fRight;
    T   fBottom;

    T left() const { return fLeft; }
    T top() const { return fTop; }
    T right() const { return fRight; }
    T bottom() const { return fBottom; }

    T x() const { return fLeft; }
    T y() const { return fTop; }
    T width() const { return fRight - fLeft; }
    T height() const { return fBottom - fTop; }

    friend bool operator==(const GTRect& a, const GTRect& b) {
        return  a.fLeft == b.fLeft  && a.fTop    == b.fTop &&
               a.fRight == b.fRight && a.fBottom == b.fBottom;
    }
    friend bool operator!=(const GTRect& a, const GTRect& b) {
        return !(a == b);
    }

    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }

    void setLTRB(T l, T t, T r, T b) {
        fLeft   = l;
        fTop    = t;
        fRight  = r;
        fBottom = b;
    }
    
    void setXYWH(T x, T y, T w, T h) {
        fLeft   = x;
        fTop    = y;
        fRight  = x + w;
        fBottom = y + h;
    }
    
    void setWH(T w, T h) {
        fLeft   = 0;
        fTop    = 0;
        fRight  = w;
        fBottom = h;
    }
    
    bool intersects(const GTRect& other) const {
        T l = std::max(fLeft,   other.fLeft);
        T t = std::max(fTop,    other.fTop);
        T r = std::min(fRight,  other.fRight);
        T b = std::min(fBottom, other.fBottom);
        // only return true if the resulting intersection is non-empty and non-inverted
        return l < r && t < b;
    }

    bool intersect(const GTRect& other) {
        T l = std::max(fLeft,   other.fLeft);
        T t = std::max(fTop,    other.fTop);
        T r = std::min(fRight,  other.fRight);
        T b = std::min(fBottom, other.fBottom);
        // only return true if the resulting intersection is non-empty and non-inverted
        if (l < r && t < b) {
            this->setLTRB(l, t, r, b);
            return true;
        }
        return false;
    }

    bool contains(T x, T y) const {
        return fLeft <= x && x < fRight && fTop <= y && y < fBottom;
    }

    bool contains(const GTRect& inner) const {
        return fLeft <= inner.fLeft && fTop <= inner.fTop &&
               fRight >= inner.fRight && fBottom >= inner.fBottom;
    }

    void offset(T dx, T dy) {
        fLeft   += dx;
        fTop    += dy;
        fRight  += dx;
        fBottom += dy;
    }
};

class GIRect : public GTRect<int32_t> {
public:
    static GIRect MakeLTRB(int32_t l, int32_t t, int32_t r, int32_t b) {
        GIRect rect;
        rect.setLTRB(l, t, r, b);
        return rect;
    }
    
    static GIRect MakeXYWH(int32_t x, int32_t y, int32_t w, int32_t h) {
        GIRect rect;
        rect.setXYWH(x, y, w, h);
        return rect;
    }
    
    static GIRect MakeWH(int32_t w, int32_t h) {
        GIRect rect;
        rect.setWH(w, h);
        return rect;
    }

    GIRect makeOffset(int dx, int dy) const {
        GIRect r = *this;
        r.offset(dx, dy);
        return r;
    }
};

class GRect : public GTRect<float> {
public:
    static GRect MakeLTRB(float l, float t, float r, float b) {
        GRect rect;
        rect.setLTRB(l, t, r, b);
        return rect;
    }
    
    static GRect MakeXYWH(float x, float y, float w, float h) {
        GRect rect;
        rect.setXYWH(x, y, w, h);
        return rect;
    }
    
    static GRect MakeWH(float w, float h) {
        GRect rect;
        rect.setWH(w, h);
        return rect;
    }
    
    static GRect Make(const GIRect& ir) {
        return MakeLTRB((float)ir.left(), (float)ir.top(), (float)ir.right(), (float)ir.bottom());
    }

    float centerX() const { return (fLeft + fRight) * 0.5f; }
    float centerY() const { return (fTop + fBottom) * 0.5f; }

    GRect makeOffset(float dx, float dy) const {
        GRect r = *this;
        r.offset(dx, dy);
        return r;
    }

    GIRect round() const {
        return GIRect::MakeLTRB(GRoundToInt(fLeft), GRoundToInt(fTop),
                                GRoundToInt(fRight), GRoundToInt(fBottom));
    }

    GIRect roundOut() const {
        return GIRect::MakeLTRB(GFloorToInt(fLeft), GFloorToInt(fTop),
                                GCeilToInt(fRight), GCeilToInt(fBottom));
    }
};

#endif
