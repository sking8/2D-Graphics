/**
 *  Copyright 2018 Mike Reed
 */

#include "GPath.h"

template <typename DRAW> void spin(GCanvas* canvas, int N, DRAW draw) {
    for (int i = 0; i < N; ++i) {
        canvas->save();
        canvas->rotate(2 * M_PI * i / N);
        draw(canvas);
        canvas->restore();
    }
}

static void show_spiro(GCanvas* canvas, const GPoint pts[4]) {
    GRandom rand;
    GPath path;
    path.moveTo(pts[0]).cubicTo(pts[1], pts[2], pts[3]);

    canvas->save();
    canvas->scale(0.5f, 0.5f);
    canvas->translate(900, 700);
    spin(canvas, 9, [&path, &rand](GCanvas* canvas) {
        canvas->drawPath(path, GPaint({1, rand.nextF(), rand.nextF(), rand.nextF()}));
    });
    canvas->restore();
}

class CubicShape : public Shape {
    GPoint fPts[4];
    bool fShowHalves = false;
public:
    CubicShape() {
        fPts[0] = { 20, 100 };
        fPts[1] = { 100, 20 };
        fPts[2] = { 160, 120 };
        fPts[3] = { 250, 50 };
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
        path.moveTo(fPts[0]).cubicTo(fPts[1], fPts[2], fPts[3]);
        canvas->drawPath(path, paint);

        if (false) { show_spiro(canvas, fPts); }

        if (fShowHalves) {
            GPoint dst[7];
            GPath::ChopCubicAt(fPts, dst, 0.5);
            path.reset().moveTo(dst[0]).cubicTo(dst[1], dst[2], dst[3]);
            canvas->drawPath(path, GPaint({0.5, 0, 1, 0}));
            path.reset().moveTo(dst[3]).cubicTo(dst[4], dst[5], dst[6]);
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
        return GPath().moveTo(fPts[0]).cubicTo(fPts[1], fPts[2], fPts[3]).bounds();
    }

    void setRect(const GRect& r) override {}
    GColor onGetColor() override { return fColor; }
    void onSetColor(const GColor& c) override { fColor = c; }

    GClick* findClick(GPoint p, GWindow* wind) override {
        if (GClick* click = Shape::findClick(p, wind)) {
            return click;
        }

        int index = -1;
        for (int i = 0; i < 4; ++i) {
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

