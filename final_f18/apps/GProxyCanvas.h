/*
 *  Copyright 2017 Mike Reed
 */

#ifndef GProxyCanvas_DEFINED
#define GProxyCanvas_DEFINED

#include "GCanvas.h"

class GProxyCanvas : public GCanvas {
public:
    GProxyCanvas(GCanvas* proxy) : fProxy(proxy) {}

    bool virtual allowDraw() { return true; }

    void save() override { if (fProxy) fProxy->save(); }
    void restore() override { if (fProxy) fProxy->restore(); }
    void concat(const GMatrix& m) override { if (fProxy) fProxy->concat(m); }

    void drawPaint(const GPaint& p) override {
        if (this->allowDraw()) {
            fProxy->drawPaint(p);
        }
    }

    void drawRect(const GRect& r, const GPaint& p) override {
        if (this->allowDraw()) {
            fProxy->drawRect(r, p);
        }
    }

    void drawConvexPolygon(const GPoint pts[], int count, const GPaint& p) override {
        if (this->allowDraw()) {
            fProxy->drawConvexPolygon(pts, count, p);
        }
    }

protected:
    void onSaveLayer(const GRect* bounds, const GPaint& paint) override {
        if (fProxy) { fProxy->saveLayer(bounds, paint); }
    }

private:
    GCanvas* fProxy;
};

#endif
