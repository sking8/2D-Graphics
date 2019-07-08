/**
 *  Copyright 2015 Mike Reed
 */

#include "bench.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRandom.h"
#include "GRect.h"
#include <string>

static GColor rand_color(GRandom& rand, bool forceOpaque = false) {
    GColor c { rand.nextF(), rand.nextF(), rand.nextF(), rand.nextF() };
    if (forceOpaque) {
        c.fA = 1;
    }
    return c;
}

static GRect rand_rect(GRandom& rand, const GRect& bounds) {
    const float x = bounds.width();
    const float y = bounds.height();
    float tmp[4] {
        rand.nextF() * x - bounds.left(), rand.nextF() * y - bounds.top(),
        rand.nextF() * x - bounds.left(), rand.nextF() * y - bounds.top()
    };
    return GRect::MakeXYWH(std::min(tmp[0], tmp[2]), std::min(tmp[1], tmp[3]),
                           std::max(tmp[0], tmp[2]), std::max(tmp[1], tmp[3]));
}

class RectsBench : public GBenchmark {
    enum { W = 200, H = 200 };
    const bool fForceOpaque;
public:
    RectsBench(bool forceOpaque) : fForceOpaque(forceOpaque) {}
    
    const char* name() const override { return fForceOpaque ? "rects_opaque" : "rects_blend"; }
    GISize size() const override { return { W, H }; }
    void draw(GCanvas* canvas) override {
        const int N = 500;
        const GRect bounds = GRect::MakeLTRB(-10, -10, W + 10, H + 10);
        GRandom rand;
        for (int i = 0; i < N; ++i) {
            GColor color = rand_color(rand, fForceOpaque);
            GRect rect = rand_rect(rand, bounds);
            canvas->fillRect(rect, color);
        }
    }
};

class SingleRectBench : public GBenchmark {
    const GISize    fSize;
    const GRect     fRect;
    const char*     fName;
public:
    SingleRectBench(GISize size, GRect r, const char* name) : fSize(size), fRect(r), fName(name) {}
    
    const char* name() const override { return fName; }
    GISize size() const override { return fSize; }
    void draw(GCanvas* canvas) override {
        const int N = 10000;
        GRandom rand;
        for (int i = 0; i < N; ++i) {
            GColor color = rand_color(rand);
            canvas->fillRect(fRect, color);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static void to_quad(const GRect& r, GPoint quad[4]) {
    quad[0].set(r.left(),  r.top());
    quad[1].set(r.right(), r.top());
    quad[2].set(r.right(), r.bottom());
    quad[3].set(r.left(),  r.bottom());
}

class PolyRectsBench : public GBenchmark {
    enum { W = 200, H = 200 };
    const bool fForceOpaque;
public:
    PolyRectsBench(bool forceOpaque) : fForceOpaque(forceOpaque) {}
    
    const char* name() const override { return fForceOpaque ? "quads_opaque" : "quads_blend"; }
    GISize size() const override { return { W, H }; }
    void draw(GCanvas* canvas) override {
        const int N = 500;
        const GRect bounds = GRect::MakeLTRB(-10, -10, W + 10, H + 10);
        GRandom rand;
        for (int i = 0; i < N; ++i) {
            GColor color = rand_color(rand, fForceOpaque);
            GPoint quad[4];
            to_quad(rand_rect(rand, bounds), quad);
            canvas->drawConvexPolygon(quad, 4, GPaint(color));
        }
    }
};

static void tesselate_circle(GPoint pts[], int count, float cx, float cy, float rad) {
    GASSERT(count >= 3);
    for (int i = 0; i < count; ++i) {
        float angle = i * M_PI * 2 / count;
        pts[i].set(cos(angle) * rad + cx, sin(angle) * rad + cy);
    }
}

class CirclesBench : public GBenchmark {
    enum { W = 200, H = 200 };
    const bool fTiny;
public:
    CirclesBench(bool tiny) : fTiny(tiny) {}
    
    const char* name() const override { return fTiny ? "circles_tiny" : "circles_large"; }
    GISize size() const override { return { W, H }; }
    void draw(GCanvas* canvas) override {
        GPoint circle[100];
        tesselate_circle(circle, 100, 100, 100, fTiny ? 5 : 90);

        const int N = 500;
        const GRect bounds = GRect::MakeLTRB(-10, -10, W + 10, H + 10);
        GRandom rand;
        for (int i = 0; i < N; ++i) {
            canvas->drawConvexPolygon(circle, 100, GPaint(rand_color(rand, true)));
        }
    }
};

class ModesBench : public GBenchmark {
    enum { W = 200, H = 200 };
    const GColor fColor;
    const char* fName;
public:
    ModesBench(const GColor& c, const char* name) : fColor(c), fName(name) {}

    const char* name() const override { return fName; }
    GISize size() const override { return { W, H }; }
    void draw(GCanvas* canvas) override {
        const GRect r = GRect::MakeWH(W, H);
        GPaint paint(fColor);

        const int N = 50;
        for (int m = 0; m < 12; ++m) {
            paint.setBlendMode(static_cast<GBlendMode>(m));
            for (int i = 0; i < N; ++i) {
                canvas->drawRect(r, paint);
            }
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////

const GBenchmark::Factory gBenchFactories[] {
    []() -> GBenchmark* { return new RectsBench(false); },
    []() -> GBenchmark* { return new RectsBench(true);  },
    []() -> GBenchmark* {
        return new SingleRectBench({2,2}, GRect::MakeLTRB(-1000, -1000, 1002, 1002), "rect_big");
    },
    []() -> GBenchmark* {
        return new SingleRectBench({1000,1000}, GRect::MakeLTRB(500, 500, 502, 502), "rect_tiny");
    },

    []() -> GBenchmark* { return new PolyRectsBench(false); },
    []() -> GBenchmark* { return new PolyRectsBench(true);  },
    []() -> GBenchmark* { return new CirclesBench(false); },
    []() -> GBenchmark* { return new CirclesBench(true);  },
    []() -> GBenchmark* { return new ModesBench({0.0, 1, 0.5, 0.25}, "modes_0"); },
    []() -> GBenchmark* { return new ModesBench({0.5, 1, 0.5, 0.25}, "modes_half"); },
    []() -> GBenchmark* { return new ModesBench({1.0, 1, 0.5, 0.25}, "modes_1"); },

    nullptr,
};
