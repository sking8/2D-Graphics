/**
 *  Copyright 2015 Mike Reed
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

static void draw_quad_rotate(GCanvas* canvas) {
    const GPoint pts[] {
        { 0, 0.1f }, { -1.65f, 5 }, { 0, 6 }, { 1.65f, 5 },
    };
    
    GPath path;
    path.moveTo(pts[0]).quadTo(pts[1], pts[2]).quadTo(pts[3], pts[0]);

    canvas->translate(150, 150);
    canvas->scale(25, 25);
    
    float steps = 12;
    float r = 0;
    float b = 1;
    float step = 1 / (steps - 1);
    
    GPaint paint;
    
    for (float angle = 0; angle < 2*M_PI - 0.001f; angle += 2*M_PI/steps) {
        paint.setColor({ 1, r, 0, b });
        canvas->save();
        canvas->rotate(angle);
        auto sh = GCreateLinearGradient({0,0}, {1,1}, {1,r,0,1}, {1,1,0,b}, GShader::kRepeat);
        canvas->drawPath(path, GPaint(sh.get()));
        canvas->restore();
        r += step;
        b -= step;
    }
}

template <typename DRAW> void spin(GCanvas* canvas, int N, DRAW draw) {
    for (int i = 0; i < N; ++i) {
        canvas->save();
        canvas->rotate(2 * M_PI * i / N);
        draw(canvas);
        canvas->restore();
    }
}

static void draw_cubics(GCanvas* canvas) {
    GPath path;
    path.moveTo(10, 0).cubicTo(100, 100, 100, -120, 200, 0);
    GRandom rand;
    
    auto rand_color = [&rand]() -> GColor {
        return { 1, rand.nextF(), rand.nextF(), rand.nextF() };
    };

    canvas->translate(256, 256);
    spin(canvas, 29, [&](GCanvas* canvas) {
        canvas->drawPath(path, GPaint(rand_color()));
    });
}

static GPath make_ear(float r) {
    GPath path;
    path.addCircle({0, 0}, r, GPath::kCW_Direction);
    path.addCircle({0, 0}, r * .75f, GPath::kCCW_Direction);
    return path;
}

static void draw_rings(GCanvas* canvas) {
    GPath path;
    GPaint paint;
    
    GRandom rand;
    for (int i = 0; i < 20; ++i) {
        path = make_ear(10 + rand.nextF() * 400);
        float tx = rand.nextF() * 512;
        float ty = rand.nextF() * 512;
        canvas->save();
        canvas->translate(tx, ty);
        canvas->drawPath(path, GPaint(rand_color(rand)));
        canvas->restore();
    }
}

static void draw_bm_tiling(GCanvas* canvas) {
    const GMatrix m = GMatrix::MakeScale(0.25f, 0.25f).postRotate(M_PI/6);
    GBitmap bm;
    bm.readFromFile("apps/spock.png");
    auto sh = GCreateBitmapShader(bm, m, GShader::kRepeat);
    canvas->drawRect(GRect::MakeXYWH(0, 0, 512, 250), GPaint(sh.get()));
    sh = GCreateBitmapShader(bm, m, GShader::kMirror);
    canvas->drawRect(GRect::MakeXYWH(0, 262, 512, 250), GPaint(sh.get()));
}

static void draw_cartman(GCanvas* canvas) {
    GPath path;
    GPaint paint;
#include "cartman.475"
}

static void draw_quad(GCanvas* canvas, GPoint p, const GPoint pts[3], GColor c) {
    canvas->drawPath(GPath().moveTo(p).lineTo(pts[0]).quadTo(pts[1], pts[2]), GPaint(c));
}

static void draw_cubic(GCanvas* canvas, GPoint p, const GPoint pts[4], GColor c) {
    canvas->drawPath(GPath().moveTo(p).lineTo(pts[0]).cubicTo(pts[1], pts[2], pts[3]), GPaint(c));
}

static void draw_divided(GCanvas* canvas) {
    GPoint quad[] = { { 0, 100 }, { 150, -90 }, { 300, 200 } };
    GPoint pivot = (quad[0] + quad[2]) * 0.5f;
    GPoint tmp0[7], tmp1[7], tmp2[7];

    GPath::ChopQuadAt(quad, tmp0, 0.25);
    GPath::ChopQuadAt(&tmp0[2], tmp1, 1.0f / 3);
    GPath::ChopQuadAt(&tmp1[2], tmp2, 0.5);
    draw_quad(canvas, pivot, tmp0, {1,1,0,0});
    draw_quad(canvas, pivot, tmp1, {1,0,0,1});
    draw_quad(canvas, pivot, tmp2, {1,0,1,0});
    draw_quad(canvas, pivot, &tmp2[2], {1,0.5,0.25,0.75});


    GPoint cubic[] = { { 0, 200 }, { 50, 650 }, { 0, 350 }, { 500, 250 } };
    pivot = (cubic[0] + cubic[3]) * 0.5f;
    
    GPath::ChopCubicAt(cubic, tmp0, 0.25);
    GPath::ChopCubicAt(&tmp0[3], tmp1, 1.0f / 3);
    GPath::ChopCubicAt(&tmp1[3], tmp2, 0.5);
    draw_cubic(canvas, pivot, tmp0, {1,0.5,0.25,0.75});
    draw_cubic(canvas, pivot, tmp1, {1,0,1,0});
    draw_cubic(canvas, pivot, tmp2, {1,0,0,1});
    draw_cubic(canvas, pivot, &tmp2[3], {1,1,0,0});
}
