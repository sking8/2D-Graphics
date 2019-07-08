/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GPoint.h"
#include "GRandom.h"
#include "GRect.h"
#include <string>

static void draw_solid_ramp(GCanvas* canvas) {
    const float c = 1.0 / 512;
    const float d = 1.0 / 256;

    const struct {
        GColor  fC0, fDC;
    } rec[] = {
        { GColor::MakeARGB(1,   c,   c,   c), GColor::MakeARGB(0,  d,  d,  d) },   // grey
        { GColor::MakeARGB(1, 1-c,   0,   0), GColor::MakeARGB(0, -d,  0,  0) },   // red
        { GColor::MakeARGB(1,   0,   c,   c), GColor::MakeARGB(0,  0,  d,  d) },   // cyan
        { GColor::MakeARGB(1,   0, 1-c,   0), GColor::MakeARGB(0,  0, -d,  0) },   // green
        { GColor::MakeARGB(1,   c,   0,   c), GColor::MakeARGB(0,  d,  0,  d) },   // magenta
        { GColor::MakeARGB(1,   0,   0, 1-c), GColor::MakeARGB(0,  0,  0, -d) },   // blue
        { GColor::MakeARGB(1,   c,   c,   0), GColor::MakeARGB(0,  d,  d,  0) },   // yellow
    };

    
    for (int y = 0; y < GARRAY_COUNT(rec); ++y) {
        GColor color = rec[y].fC0;
        GColor delta = rec[y].fDC;
        for (int x = 0; x < 256; x++) {
            const GRect rect = GRect::MakeXYWH(x, y * 28, 1, 28);
            canvas->fillRect(rect, color);
            color.fA += delta.fA;
            color.fR += delta.fR;
            color.fG += delta.fG;
            color.fB += delta.fB;
        }
    }
}

static void draw_blend_ramp(GCanvas* canvas, const GColor& bg) {
    GRect rect = GRect::MakeXYWH(-25, -25, 70, 70);

    int delta = 8;
    for (int i = 0; i < 200; i += delta) {
        float r = i / 200.0;
        float g = fabs(cos(i/40.0));
        float b = fabs(sin(i/50.0));
        GColor color = GColor::MakeARGB(0.3, r, g, b);
        canvas->fillRect(rect, color);
        rect.offset(delta, delta);
    }
}

static GRect rand_rect(GRandom& rand, float mx, float my) {
    float x = rand.nextF() * mx - mx / 8;
    float y = rand.nextF() * my - my / 8;
    float w = rand.nextF() * mx / 4;
    float h = rand.nextF() * my / 4;
    return GRect::MakeXYWH(x, y, w, h);
}

static GColor rand_color(GRandom& rand) {
    float a = rand.nextF();
    float r = rand.nextF();
    float g = rand.nextF();
    float b = rand.nextF();
    return { a, r, g, b };
}

struct Graph {
    int fSteps;
    float fMin, fMax;
    GPoint fScale = { 1, 1 };
    GPoint fOffset = { 0, 0 };
};

static void graph_rects(GCanvas* canvas, const Graph& g,
                        float (*func)(float x), GColor (*color)(float t)) {
    const float dx = (g.fMax - g.fMin) / (g.fSteps - 1);
    const float halfwidth = dx * g.fScale.fX * 0.5f;
    const float y0 = g.fOffset.fY;
    float x = g.fMin;
    for (int i = 0; i < g.fSteps; ++i) {
        float y = func(x);
        float sx = x * g.fScale.fX + g.fOffset.fX;
        float sy = -y * g.fScale.fY + g.fOffset.fY;
        GRect r = GRect::MakeLTRB(sx - halfwidth, sy, sx + halfwidth, y0);
        if (r.height() < 0) {
            std::swap(r.fTop, r.fBottom);
        }
        r.fLeft += 0.5;
        r.fRight -= 0.5f;
        canvas->fillRect(r, color((x - g.fMin) / (g.fMax - g.fMin)));
        x += dx;
    }
}

static void draw_graphs(GCanvas* canvas) {
    Graph g;
    g.fSteps = 30;
    g.fMin = -M_PI;
    g.fMax = M_PI;
    g.fScale = { 40, 60 };
    g.fOffset = { 128, 128 };
    graph_rects(canvas, g, [](float x) {
                    return sinf(x);
                },
                [](float t) -> GColor {
                    return { 1, t, 0, 1 - t };
                });

    g.fSteps = 70;
    g.fMin = -5*M_PI;
    g.fMax =  5*M_PI;
    g.fScale = { 10, 40 };
    g.fOffset = { 128, 40 };
    graph_rects(canvas, g, [](float x) {
                    return sinf(x) / x;
                },
                [](float t) -> GColor { return { 1, 0, 0.5, 0 }; }
                );

    g.fSteps = 20;
    g.fMin = 0;
    g.fMax = 1;
    g.fScale = { 100, 100 };
    g.fOffset = { 128, 250 };
    graph_rects(canvas, g,
                [](float x) { return sqrtf(x); },
                [](float t) -> GColor { float c = (1 - t) * 0.75f; return { 1, c, c, c }; }
                );
}

static void draw_blend_black(GCanvas* canvas) {
    draw_blend_ramp(canvas, GColor::MakeARGB(1, 0, 0, 0));
}
