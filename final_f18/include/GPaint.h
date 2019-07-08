/*
 *  Copyright 2016 Mike Reed
 */

#ifndef GPaint_DEFINED
#define GPaint_DEFINED

#include "GColor.h"
#include "GBlendMode.h"

class GShader;

class GPaint {
public:
    GPaint() {}
    GPaint(const GColor& c) : fColor(c) {}
    GPaint(GShader* sh) : fShader(sh) {}

    const GColor& getColor() const { return fColor; }
    GPaint& setColor(GColor c) { fColor = c; return *this; }

    float getAlpha() const { return fColor.fA; }
    GPaint& setAlpha(float alpha) {
        fColor.fA = alpha;
        return *this;
    }

    GBlendMode getBlendMode() const { return fMode; }
    GPaint& setBlendMode(GBlendMode m) { fMode = m; return *this; }

    GShader* getShader() const { return fShader; }
    GPaint&  setShader(GShader* s) { fShader = s; return *this; }

private:
    GColor      fColor = GColor::MakeARGB(1, 0, 0, 0);
    GShader*    fShader = nullptr;
    GBlendMode  fMode = GBlendMode::kSrcOver;
};

#endif
