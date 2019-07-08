/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GPoint_DEFINED
#define GPoint_DEFINED

#include "GMath.h"

class GPoint;

struct GVector {
    float fX;
    float fY;

    float x() const { return fX; }
    float y() const { return fY; }
    float length() const { return sqrtf(fX*fX + fY*fY); }

    void set(float x, float y) {
        fX = x;
        fY = y;
    }

    GVector operator+(const GVector& v) const {
        return { fX + v.fX, fY + v.fY };
    }
    GVector operator-(const GVector& v) const {
        return { fX - v.fX, fY - v.fY };
    }

    friend GVector operator*(GVector v, float s) {
        return { v.fX * s, v.fY * s };
    }
    friend GVector operator*(float s, GVector v) {
        return { v.fX * s, v.fY * s };
    }

    inline operator GPoint() const;
};

class GPoint {
public:
    float fX;
    float fY;

    float x() const { return fX; }
    float y() const { return fY; }

    void set(float x, float y) {
        fX = x;
        fY = y;
    }

    bool operator==(const GPoint& p) const {
        return fX == p.fX && fY == p.fY;
    }
    bool operator!=(const GPoint& p) const { return !(*this == p); }

    static GPoint Make(float x, float y) {
        GPoint pt = { x, y };
        return pt;
    }

    GPoint operator+(const GVector& v) const {
        return { fX + v.fX, fY + v.fY };
    }
    GPoint operator-(const GVector& v) const {
        return { fX - v.fX, fY - v.fY };
    }
    GPoint& operator+=(const GVector& v) {
        *this = *this + v;
        return *this;
    }

    GVector operator-(const GPoint& p) const {
        return { fX - p.fX, fY - p.fY };
    }

    GPoint operator+(const GPoint& p) const {
        return { fX + p.fX, fY + p.fY };
    }

    friend GPoint operator*(GPoint v, float s) {
        return { v.fX * s, v.fY * s };
    }
    friend GPoint operator*(float s, GPoint v) {
        return { v.fX * s, v.fY * s };
    }
};

inline GVector::operator GPoint() const { return GPoint{ fX, fY }; }

template <typename T> class GSize {
public:
    T fWidth, fHeight;
    
    T width() { return fWidth; }
    T height() { return fHeight; }
};
typedef GSize<int> GISize;

#endif
