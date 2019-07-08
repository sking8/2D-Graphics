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

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
    float angle = 0;
    const float deltaAngle = M_PI * 2 / count;

    for (int i = 0; i < count; ++i) {
        pts[i].set(cx + cos(angle) * radius, cy + sin(angle) * radius);
        angle += deltaAngle;
    }
}

static void dr_poly(GCanvas* canvas, float dx, float dy) {
    GPoint storage[12];
    for (int count = 12; count >= 3; --count) {
        make_regular_poly(storage, count, 256, 256, count * 10 + 120);
        for (int i = 0; i < count; ++i) {
            storage[i].fX += dx;
            storage[i].fY += dy;
        }
        GColor c = GColor::MakeARGB(0.8f,
                                    fabs(sin(count*7)),
                                    fabs(sin(count*11)),
                                    fabs(sin(count*17)));
        canvas->drawConvexPolygon(storage, count, GPaint(c));
    }
}

static void draw_poly(GCanvas* canvas) {
    dr_poly(canvas, 0, 0);
}

static void draw_poly_center(GCanvas* canvas) {
    dr_poly(canvas, -128, -128);
}

static void draw_poly_vstrip(GCanvas* canvas) {
    dr_poly(canvas, -256, 0);
}

static GPoint scale(GPoint vec, float size) {
    float scale = size / sqrt(vec.fX * vec.fX + vec.fY * vec.fY);
    return GPoint::Make(vec.fX * scale, vec.fY * scale);
}

static void draw_line(GCanvas* canvas, GPoint a, GPoint b, float width, const GColor& color) {
    GPoint norm = scale(GPoint::Make(b.fY - a.fY, a.fX - b.fX), width/2);

    GPoint pts[4];
    pts[0] = GPoint::Make(a.fX + norm.fX, a.fY + norm.fY);
    pts[1] = GPoint::Make(b.fX + norm.fX, b.fY + norm.fY);
    pts[2] = GPoint::Make(b.fX - norm.fX, b.fY - norm.fY);
    pts[3] = GPoint::Make(a.fX - norm.fX, a.fY - norm.fY);

    canvas->drawConvexPolygon(pts, 4, GPaint(color));
}

////////////////////////////////////////////////////////////////////////////////////

static void outer_frame(GCanvas* canvas, const GRect& r) {
    GPaint paint;
    canvas->drawRect(GRect::MakeXYWH(r.fLeft - 2, r.fTop - 2, 1, r.height() + 4), paint);
    canvas->drawRect(GRect::MakeXYWH(r.fRight + 1, r.fTop - 2, 1, r.height() + 4), paint);
    canvas->drawRect(GRect::MakeXYWH(r.fLeft - 1, r.fTop - 2, r.width() + 2, 1), paint);
    canvas->drawRect(GRect::MakeXYWH(r.fLeft - 1, r.fBottom + 1, r.width() + 2, 1), paint);
}

// so we test the polygon code
static GPoint* rect_pts(const GRect& r, GPoint pts[]) {
    pts[0] = { r.fLeft,  r.fTop };
    pts[1] = { r.fRight, r.fTop };
    pts[2] = { r.fRight, r.fBottom };
    pts[3] = { r.fLeft,  r.fBottom };
    return pts;
}

static void draw_mode_sample(GCanvas* canvas, const GRect& bounds, GBlendMode mode) {
    const float dx = bounds.width() / 3;
    const float dy = bounds.height() / 3;

    outer_frame(canvas, bounds);

    GPaint paint;
    GPoint pts[4];

    // dst is red
    paint.setBlendMode(GBlendMode::kSrc);
    GRect r = bounds;
    r.fBottom = r.fTop + dy;
    canvas->drawConvexPolygon(rect_pts(r, pts), 4, paint.setColor({0, 0, 0, 0}));
    r.offset(0, dy);
    canvas->drawConvexPolygon(rect_pts(r, pts), 4, paint.setColor({0.5, 1, 0, 0}));
    r.offset(0, dy);
    canvas->drawConvexPolygon(rect_pts(r, pts), 4, paint.setColor({1, 1, 0, 0}));

    // src is blue
    paint.setBlendMode(mode);
    r = bounds;
    r.fRight = r.fLeft + dx;
    canvas->drawRect(r, paint.setColor({0, 0, 0, 0}));
    r.offset(dx, 0);
    canvas->drawRect(r, paint.setColor({0.5, 0, 0, 1}));
    r.offset(dx, 0);
    canvas->drawRect(r, paint.setColor({1, 0, 0, 1}));
}

static void draw_blendmodes(GCanvas* canvas) {
    canvas->clear({1,1,1,1});

    const float W = 100;
    const float H = 100;
    const float margin = 10;
    float x = margin;
    float y = margin;
    for (int i = 0; i < 12; ++i) {
        GBlendMode mode = static_cast<GBlendMode>((i + 1) % 12);
        draw_mode_sample(canvas, GRect::MakeXYWH(x, y, W, H), mode);
        if (i % 4 == 3) {
            y += H + margin;
            x = margin;
        } else {
            x += W + margin;
        }
    }
}
