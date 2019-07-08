/**
 *  Copyright 2018 Mike Reed
 */

#include "GPath.h"

class QuadShape : public Shape {
    GPoint fPts[3];
    bool fShowHalves = false;
public:
    QuadShape() {
        fPts[0] = { 20, 100 };
        fPts[1] = { 100, 20 };
        fPts[2] = { 160, 120 };
        fColor = { 1, 1, 0, 0 };
    }

    bool handleSym(uint32_t sym) override {
        if (sym == 'h') {
            fShowHalves = !fShowHalves;
            return true;
        }
        return this->Shape::handleSym(sym);
    }

    void onDraw(GCanvas* canvas, const GPaint& paint) override {
        GPath path;
        path.moveTo(fPts[0]).quadTo(fPts[1], fPts[2]);
        canvas->drawPath(path, paint);
        if (fShowHalves) {
            GPoint dst[5];
            GPath::ChopQuadAt(fPts, dst, 0.5);
            path.reset().moveTo(dst[0]).quadTo(dst[1], dst[2]);
            canvas->drawPath(path, GPaint({0.5, 0, 1, 0}));
            path.reset().moveTo(dst[2]).quadTo(dst[3], dst[4]);
            canvas->drawPath(path, GPaint({0.5, 0, 0, 1}));
        }
    }

    void drawHilite(GCanvas* canvas) override {
        GPath path;
        for (auto p : fPts) {
            path.addCircle(p, 6);
        }
        canvas->drawPath(path, GPaint());

        this->Shape::drawGradientHilite(canvas);
    }

    GRect getRect() override {
        return GPath().moveTo(fPts[0]).quadTo(fPts[1], fPts[2]).bounds();
    }

    void setRect(const GRect& r) override {}
    GColor onGetColor() override { return fColor; }
    void onSetColor(const GColor& c) override { fColor = c; }

    GClick* findClick(GPoint p, GWindow* wind) override {
        if (GClick* click = Shape::findClick(p, wind)) {
            return click;
        }

        int index = -1;
        for (int i = 0; i < 3; ++i) {
            if (hit_test(p.x(), p.y(), fPts[i].x(), fPts[i].y())) {
                index = i;
            }
        }
        if (index >= 0) {
            return new GClick(p, [this, wind, index](GClick* click) {
                fPts[index] = click->curr();
                wind->requestDraw();
            });
        }
        return nullptr;
    }
private:
    GColor  fColor;
};

