/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GMatrix_DEFINED
#define GMatrix_DEFINED

#include "GColor.h"
#include "GMath.h"
#include "GPoint.h"
#include "GRect.h"

class GMatrix {
public:
    GMatrix() { this->setIdentity(); }
    GMatrix(float a, float b, float c, float d, float e, float f) {
        fMat[0] = a;    fMat[1] = b;    fMat[2] = c;
        fMat[3] = d;    fMat[4] = e;    fMat[5] = f;
    }

    void set6(float a, float b, float c, float d, float e, float f) {
        fMat[0] = a;    fMat[1] = b;    fMat[2] = c;
        fMat[3] = d;    fMat[4] = e;    fMat[5] = f;
    }

    enum {
        SX, KX, TX,
        KY, SY, TY,
    };
    float operator[](int index) const {
        GASSERT(index >= 0 && index < 6);
        return fMat[index];
    }

    bool operator==(const GMatrix& m) {
        for (int i = 0; i < 6; ++i) {
            if (fMat[i] != m.fMat[i]) {
                return false;
            }
        }
        return true;
    }

    // These 7 methods must be implemented by the student.

    /**
     *  Set this matrix to identity.
     */
    void setIdentity();

    /**
     *  Set this matrix to translate by the specified amounts.
     */
    void setTranslate(float tx, float ty);

    /**
     *  Set this matrix to scale by the specified amounts.
     */
    void setScale(float sx, float sy);

    /**
     *  Set this matrix to rotate by the specified radians.
     *
     *  Note: since positive-Y goes down, a small angle of rotation will increase Y.
     */
    void setRotate(float radians);

    /**
     *  Set this matrix to the concatenation of the two specified matrices, such that the resulting
     *  matrix, when applied to points will have the same effect as first applying the primo matrix
     *  to the points, and then applying the secundo matrix to the resulting points.
     *
     *  Pts' = Secundo * Primo * Pts
     */
    void setConcat(const GMatrix& secundo, const GMatrix& primo);

    /*
     *  If this matrix is invertible, return true and (if not null) set the inverse parameter.
     *  If this matrix is not invertible, return false and ignore the inverse parameter.
     */
    bool invert(GMatrix* inverse) const;
    
    /**
     *  Transform the set of points in src, storing the resulting points in dst, by applying this
     *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
     *
     *  Note: It is legal for src and dst to point to the same memory (however, they may not
     *  partially overlap). Thus the following is supported.
     *
     *  GPoint pts[] = { ... };
     *  matrix.mapPoints(pts, pts, count);
     */
    void mapPoints(GPoint dst[], const GPoint src[], int count) const;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // These helper methods are implemented in terms of the previous 7 methods.

    GMatrix& preConcat(const GMatrix& primo) {
        this->setConcat(*this, primo);
        return *this;
    }

    GMatrix& preTranslate(float x, float y) {
        GMatrix trans;
        trans.setTranslate(x, y);
        return this->preConcat(trans);
    }

    GMatrix& preScale(float sx, float sy) {
        GMatrix scale;
        scale.setScale(sx, sy);
        return this->preConcat(scale);
    }

    GMatrix& preRotate(float radians) {
        GMatrix rotate;
        rotate.setRotate(radians);
        return this->preConcat(rotate);
    }

    GMatrix& postConcat(const GMatrix& secundo) {
        this->setConcat(secundo, *this);
        return *this;
    }
    
    GMatrix& postTranslate(float x, float y) {
        GMatrix trans;
        trans.setTranslate(x, y);
        return this->postConcat(trans);
    }
    
    GMatrix& postScale(float sx, float sy) {
        GMatrix scale;
        scale.setScale(sx, sy);
        return this->postConcat(scale);
    }
    
    GMatrix& postRotate(float radians) {
        GMatrix rotate;
        rotate.setRotate(radians);
        return this->postConcat(rotate);
    }

    static GMatrix MakeTranslate(float tx, float ty) {
        GMatrix m;
        m.setTranslate(tx, ty);
        return m;
    }
    
    static GMatrix MakeScale(float scale) {
        GMatrix m;
        m.setScale(scale, scale);
        return m;
    }
    
    static GMatrix MakeScale(float sx, float sy) {
        GMatrix m;
        m.setScale(sx, sy);
        return m;
    }
    
    static GMatrix MakeRotate(float radians) {
        GMatrix m;
        m.setRotate(radians);
        return m;
    }
    
    void mapPoints(GPoint pts[], int count) const {
        this->mapPoints(pts, pts, count);
    }
    
    GPoint mapXY(float x, float y) const {
        GPoint pts[1]{ x, y };
        this->mapPoints(pts, 1);
        return pts[0];
    }

    GPoint mapPt(GPoint pt) const {
        this->mapPoints(&pt, 1);
        return pt;
    }

private:
    float fMat[6];
};

#endif
