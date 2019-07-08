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
#include "GShader.h"
#include <string>

class CheckerShader : public GShader {
    const GPixel fP0, fP1;
    const GMatrix fLocalMatrix;
    
    GMatrix fInverse;
    
public:
    CheckerShader(float scale, GPixel p0, GPixel p1)
        : fLocalMatrix(GMatrix::MakeScale(scale, scale))
        , fP0(p0)
        , fP1(p1)
    {}
    
    bool isOpaque() override {
        return GPixel_GetA(fP0) == 0xFF && GPixel_GetA(fP1) == 0xFF;
    }
    
    bool setContext(const GMatrix& ctm) override {
        GMatrix m;
        m.setConcat(ctm, fLocalMatrix);
        return m.invert(&fInverse);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        const float dx = fInverse[GMatrix::SX];
        const float dy = fInverse[GMatrix::KY];
        GPoint loc = fInverse.mapXY(x + 0.5f, y + 0.5f);
        
        const GPixel array[] = { fP0, fP1 };
        for (int i = 0; i < count; ++i) {
            row[i] = array[((int)loc.fX + (int)loc.fY) & 1];
            loc.fX += dx;
            loc.fY += dy;
        }
    }
};

static void draw_checker(GCanvas* canvas) {
    CheckerShader shader(20,
                         GPixel_PackARGB(0xFF, 0, 0, 0),
                         GPixel_PackARGB(0xFF, 0xFF, 0xFF, 0xFF));
    GPaint paint(&shader);
    
    canvas->clear({ 1, 0.75, 0.75, 0.75 });
    canvas->drawRect(GRect::MakeXYWH(20, 20, 100, 100), paint);
    
    canvas->save();
    canvas->translate(130, 175);
    canvas->rotate(-M_PI/3);
    canvas->drawRect(GRect::MakeXYWH(20, 20, 100, 100), paint);
    canvas->restore();
    
    canvas->save();
    canvas->translate(10, 160);
    canvas->scale(0.5, 0.5);
    canvas->drawRect(GRect::MakeXYWH(20, 20, 200, 200), paint);
    canvas->restore();
    
    CheckerShader shader2(150,
                          GPixel_PackARGB(0x44, 0x44, 0, 0),
                          GPixel_PackARGB(0x44, 0, 0, 0x44));
    paint.setShader(&shader2);
    canvas->drawRect(GRect::MakeXYWH(0, 0, 300, 300), paint);
}

static void draw_poly_rotate(GCanvas* canvas) {
    const GPoint pts[] {
        { 0, 0.1f }, { -1, 5 }, { 0, 6 }, { 1, 5 },
    };
    
    canvas->translate(150, 150);
    canvas->scale(25, 25);
    
    float steps = 12;
    float r = 0;
    float b = 1;
    float step = 1 / (steps - 1);
    
    GPaint paint;
    std::unique_ptr<GShader> shader;
    
    const GBitmap* tex = nullptr;
    if (tex) {
        shader = GCreateBitmapShader(*tex, GMatrix::MakeScale(50, 50));
        paint.setShader(shader.get());
    }
    
    for (float angle = 0; angle < 2*M_PI - 0.001f; angle += 2*M_PI/steps) {
        paint.setColor({ 1, r, 0, b });
        canvas->save();
        canvas->rotate(angle);
        canvas->drawConvexPolygon(pts, 4, paint);
        canvas->restore();
        r += step;
        b -= step;
    }
}

static void draw_clock_bm(GCanvas* canvas) {
    GBitmap bm;
    bm.readFromFile("apps/spock.png");

    float cx = bm.width() * 0.5f;
    float cy = bm.height() * 0.5f;
    GPoint pts[] = {
        { cx, 0 }, { 0, cy }, { cx, bm.height()*1.0f }, { bm.width()*1.0f, cy },
    };

    auto shader = GCreateBitmapShader(bm, GMatrix());
    GPaint paint(shader.get());

    int n = 7;
    canvas->scale(0.4f, 0.4f);
    for (int i = 0; i < n; ++i) {
        float radians = i * M_PI * 2 / n;
        canvas->save();
        canvas->translate(cx*3, cx*3);
        canvas->rotate(radians);
        canvas->translate(cx, -cy);
        canvas->drawConvexPolygon(pts, 4, paint);
        canvas->restore();
    }
}

static void draw_bitmap(GCanvas* canvas, const GRect& r, const GBitmap& bm, GBlendMode mode) {
    auto sh = GCreateBitmapShader(bm, GMatrix());
    GPaint paint;
    paint.setShader(sh.get());
    paint.setBlendMode(mode);
    
    GMatrix m = GMatrix::MakeScale(r.width() / bm.width(), r.height() / bm.height());
    m.postTranslate(r.left(), r.top());
    sh = GCreateBitmapShader(bm, m);
    paint.setShader(sh.get());
    canvas->drawRect(r, paint);
}

static void draw_bitmaps_hole(GCanvas* canvas) {
    GBitmap bm0, bm1;
    
    bm0.readFromFile("apps/spock.png");
    bm1.readFromFile("apps/oldwell.png");

    const GRect r = GRect::MakeWH(300, 300);
    draw_bitmap(canvas, r, bm0, GBlendMode::kSrc);
    draw_bitmap(canvas, r, bm1, GBlendMode::kDstIn);
}

static void draw_mode_sample2(GCanvas* canvas, const GRect& bounds, GBlendMode mode) {
    outer_frame(canvas, bounds);

    GPaint paint;
    GPoint pts[4];

    GPixel dstp[5] = {
        GPixel_PackARGB(0, 0, 0, 0),
        GPixel_PackARGB(0x40, 0x40, 0, 0),
        GPixel_PackARGB(0x80, 0x80, 0, 0),
        GPixel_PackARGB(0xC0, 0xC0, 0, 0),
        GPixel_PackARGB(0xFF, 0xFF, 0, 0),
    };
    GBitmap dstbm(1, 5, 1*4, dstp, false);
    auto dstsh = GCreateBitmapShader(dstbm, GMatrix::MakeScale(bounds.width(),
                                                               bounds.height()/5.0f));

    GPixel srcp[5] = {
        GPixel_PackARGB(0, 0, 0, 0),
        GPixel_PackARGB(0x40, 0, 0, 0x40),
        GPixel_PackARGB(0x80, 0, 0, 0x80),
        GPixel_PackARGB(0xC0, 0, 0, 0xC0),
        GPixel_PackARGB(0xFF, 0, 0, 0xFF),
    };
    GBitmap srcbm(5, 1, 5*4, srcp, false);
    auto srcsh = GCreateBitmapShader(srcbm, GMatrix::MakeScale(bounds.width()/5.0f,
                                                               bounds.height()));

    paint.setBlendMode(GBlendMode::kSrc);
    paint.setShader(dstsh.get());
    canvas->drawConvexPolygon(rect_pts(bounds, pts), 4, paint);

    paint.setBlendMode(mode);
    paint.setShader(srcsh.get());
    canvas->drawRect(bounds, paint);
}

static void draw_all_blendmodes(GCanvas* canvas, void (*proc)(GCanvas*, const GRect&, GBlendMode)) {
    canvas->clear({1,1,1,1});
    const GRect r = GRect::MakeWH(100, 100);
    
    const float W = 100;
    const float H = 100;
    const float margin = 10;
    float x = margin;
    float y = margin;
    for (int i = 0; i < 12; ++i) {
        GBlendMode mode = static_cast<GBlendMode>((i + 1) % 12);
        canvas->save();
        canvas->translate(x, y);
        proc(canvas, r, mode);
        canvas->restore();
        if (i % 4 == 3) {
            y += H + margin;
            x = margin;
        } else {
            x += W + margin;
        }
    }
}

static void draw_bm_blendmodes(GCanvas* canvas) {
    draw_all_blendmodes(canvas, draw_mode_sample2);
}
