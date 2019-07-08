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

static void draw_tri(GCanvas* canvas) {
    const GPoint pts[] = {
        { 10, 10 }, { 400, 100 }, { 250, 400 },
    };
    const GColor clr[] = {
        { 1, 1, 0, 0 }, { 1, 0, 1, 0 }, { 1, 0, 0, 1 },
    };
    const int indices[] = {
        0, 1, 2,
    };

    canvas->translate(250, 250);
    canvas->rotate(M_PI/2);
    canvas->translate(-250, -250);
    canvas->drawMesh(pts, clr, nullptr, 1, indices, GPaint());
}

static void draw_tri2(GCanvas* canvas) {
    GBitmap bm;
    bm.readFromFile("apps/spock.png");
    float w = bm.width();
    float h = bm.height();

    const GPoint pts[] = {
        { 10, 10 }, { 400, 100 }, { 250, 400 },
    };
    const GPoint tex[] = {
        { 0, 0 }, { w, 0 }, { 0, h },
    };
    const int indices[] = {
        0, 1, 2,
    };

    auto sh = GCreateBitmapShader(bm, GMatrix());
    GPaint paint(sh.get());

    canvas->translate(250, 250);
    canvas->rotate(M_PI/2);
    canvas->translate(-250, -250);
    canvas->drawMesh(pts, nullptr, tex, 1, indices, paint);
}

const float twopi = (float)(2*M_PI);

static void circular_mesh(GCanvas* canvas, bool showColors, bool showTex) {
    auto shader = GCreateLinearGradient({0, 0}, {1,0}, {1, 0,0,0}, {1, 1,1,1});

    const int TRIS = 40;
    GPoint pts[TRIS + 1];
    GColor colors[TRIS + 1];
    GPoint tex[TRIS + 1];
    int indices[TRIS * 3];

    const float rad = 250;
    const float center = 256;
    pts[0] = { center, center };
    colors[0] = { 1, 0, 1, 0 };
    tex[0] = { 1, 1 };

    float angle = 0;
    float da = twopi / (TRIS - 1);
    int* idx = indices;
    for (int i = 1; i <= TRIS; ++i) {
        float x = cos(angle);
        float y = sin(angle);
        pts[i] = { x * rad + center, y * rad + center };
        colors[i] = { 1, angle / twopi, 0, (twopi - angle) / twopi };
        tex[i] = { angle / twopi, 0 };
        idx[0] = 0; idx[1] = i; idx[2] = i < TRIS ? i + 1 : 1;
        idx += 3;
        angle += da;
    }

    const GColor* colorPtr = showColors ? colors : nullptr;
    const GPoint* texPtr = showTex ? tex : nullptr;
    canvas->drawMesh(pts, colorPtr, texPtr, TRIS, indices, GPaint(shader.get()));
}

static void mesh_1(GCanvas* canvas) { circular_mesh(canvas, true, false); }
static void mesh_3(GCanvas* canvas) { circular_mesh(canvas, true, true); }

#include "GRandom.h"
static void perterb(GPoint pts[], int count) {
    GRandom rand;
    const float s = 5;

    for (int i = 0; i < count; ++i) {
        pts[i].fX += rand.nextRange(-s, s);
        pts[i].fY += rand.nextRange(-s, s);
    }
}

static void spock_quad(GCanvas* canvas) {
    GBitmap bitmap;
    bitmap.readFromFile("apps/spock.png");
    const float w = bitmap.width();
    const float h = bitmap.height();
    auto shader = GCreateBitmapShader(bitmap, GMatrix::MakeScale(1/w, 1/h), GShader::kMirror);

    GPoint outer[6], inner[6];
    for (int i = 0; i < 6; ++i) {
        float x = cosf(i * M_PI / 3);
        float y = sinf(i * M_PI / 3);
        outer[i] = { x * 250, y * 250 };
        inner[i] = { x *  50, y *  50 };
    }

    auto six = [](int index) { return index % 6; };

    canvas->translate(256, 256);
    for (int i = 0; i < 6; ++i) {
        GPoint pts[] = { outer[six(i)], outer[six(i+1)], inner[six(i+1)], inner[six(i)] };
        GPoint tex[] = {
            { i+0.0f, 0.0f }, { i+1.0f, 0.0f }, { i+1.0f, 1.0f }, { i+0.0f, 1.0f },
        };
        canvas->drawQuad(pts, nullptr, tex, 8, GPaint(shader.get()));
    }
}

static void color_quad(GCanvas* canvas) {
    const GColor r  = { 1, 1, 0, 0 };
    const GColor rg = { 1, 1, 1, 0 };
    const GColor g  = { 1, 0, 1, 0 };
    const GColor gb = { 1, 0, 1, 1 };
    const GColor b  = { 1, 0, 0, 1 };
    const GColor br = { 1, 1, 0, 1 };
    const GColor ring[] = { r, rg, g, gb, b, br };

    GPoint outer[6], inner[6];
    for (int i = 0; i < 6; ++i) {
        float x = cosf(i * M_PI / 3);
        float y = sinf(i * M_PI / 3);
        outer[i] = { x * 250, y * 250 };
        inner[i] = { x *  50, y *  50 };
    }

    auto six = [](int index) { return index % 6; };

    canvas->translate(256, 256);
    for (int i = 0; i < 6; ++i) {
        GPoint pts[] = { outer[six(i)], outer[six(i+1)], inner[six(i+1)], inner[six(i)] };
        GColor clr[] = { ring[six(i)], ring[six(i+1)], ring[six(i+4)], ring[six(i+3)] };
        canvas->drawQuad(pts, clr, nullptr, 8, GPaint());
    }
}
