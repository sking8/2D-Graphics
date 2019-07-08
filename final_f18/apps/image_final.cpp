/**
 *  Copyright 2018 Mike Reed
 */

#include "image.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GMatrix.h"
#include "GPath.h"
#include "GPoint.h"
#include "GRandom.h"
#include "GRect.h"
#include <string>

static void draw_rect_radial(GCanvas* canvas, const GRect& r, const GColor colors[], int count) {
    float rad = (r.width() / 2) * sqrt(2);
    auto sh = canvas->final_createRadialGradient({r.centerX(), r.centerY()}, rad, colors, count,
                                                 GShader::kClamp);
    canvas->drawRect(r, GPaint(sh.get()));
}

static void final_radial(GCanvas* canvas) {
    const GColor c0[] = {
        {1,0,0,0}, {1, 0, 0.75, 0}, {0,1,1,0},
    };
    draw_rect_radial(canvas, GRect::MakeLTRB(0, 0, 512, 512), c0, GARRAY_COUNT(c0));
    const GColor c1[] = {
        {1,1,0,0}, {1, 0, 1, 0}, {1,0,0,1}, {1,1,0,0},
    };
    draw_rect_radial(canvas, GRect::MakeLTRB(-150, -150, 150, 150), c1, GARRAY_COUNT(c1));
}

static void draw_stroke(GCanvas* canvas, GPoint p0, GPoint p1, float width, bool roundCap,
                        const GPaint& paint) {
    GPath path;
    canvas->final_strokeLine(&path, p0, p1, width, roundCap);
    canvas->drawPath(path, paint);
}

static void final_stroke(GCanvas* canvas) {
    GRandom rand;

    for (int i = 0; i < 70; ++i) {
        float x0 = rand.nextF() * 512;
        float r = rand.nextF();
        float y0 = rand.nextF() * 512;
        float g = rand.nextF();
        float x1 = rand.nextF() * 512;
        float b = rand.nextF();
        float y1 = rand.nextF() * 512;
        float width = 1 + rand.nextF() * 50;
        bool roundCap = rand.nextU() & 1;
        draw_stroke(canvas, {x0, y0}, {x1, y1}, width, roundCap, GPaint({1, r, g, b}));
    }
}
