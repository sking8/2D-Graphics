/**
 *  Copyright 2018 Mike Reed
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GRandom.h"
#include "GRect.h"
#include "GShader.h"

#include <vector>

static GRandom gRand;

static GColor rand_color() {
    return GColor::MakeARGB(0.5f, gRand.nextF(), gRand.nextF(), gRand.nextF());
}

class Brush {
public:
    virtual ~Brush() {}

    virtual void draw(GCanvas*, GPoint loc) = 0;
};

static GRect offset(const GRect& r, GPoint offset) {
    return GRect::MakeXYWH(r.left() + offset.x(), r.top() + offset.y(), r.width(), r.height());
}

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
    float angle = 0;
    const float deltaAngle = M_PI * 2 / count;

    for (int i = 0; i < count; ++i) {
        pts[i].set(cx + cos(angle) * radius, cy + sin(angle) * radius);
        angle += deltaAngle;
    }
}


class RectBrush : public Brush {
public:
    RectBrush(const GRect& r, const GColor& c, GBlendMode m) : fR(r) {
        fP.setColor(c);
        fP.setBlendMode(m);
    }

    void draw(GCanvas* canvas, GPoint loc) override {
        if (false) {
        GPoint pts[12];
        make_regular_poly(pts, GARRAY_COUNT(pts), loc.x(), loc.y(), fR.width()*0.5f);
        canvas->drawConvexPolygon(pts, GARRAY_COUNT(pts), fP);
        }
        if (true) {
            canvas->drawRect(offset(fR, loc), fP);
        }
    }

private:
    GRect   fR;
    GPaint  fP;
};

static void fill(const GBitmap& bm, uint8_t byte) {
    memset(bm.getAddr(0, 0), byte, bm.height() * bm.rowBytes());
}

static void draw_bitmap(GCanvas* canvas, const GBitmap& bm, GBlendMode mode) {
    auto sh = GCreateBitmapShader(bm, GMatrix());
    GPaint paint;
    paint.setShader(sh.get());
    paint.setBlendMode(mode);
    canvas->drawRect(GRect::MakeWH(bm.width(), bm.height()), paint);
}

static void draw_bitmap(const GBitmap& bg, const GBitmap& fg, GBlendMode mode) {
    draw_bitmap(GCreateCanvas(bg).get(), fg, mode);
}

class TestWindow : public GWindow {
    GBitmap fFG, fBG;

public:
    TestWindow(int w, int h) : GWindow(w, h) {
        fFG.alloc(1024, 768); fill(fFG, 0);
        fBG.alloc(1024, 768); fill(fBG, 0xFF);
    }

    virtual ~TestWindow() {}
    
protected:
    void onDraw(GCanvas* canvas) override {
        draw_bitmap(canvas, fBG, GBlendMode::kSrc);
        draw_bitmap(canvas, fFG, GBlendMode::kSrcOver);
    }

    bool onKeyPress(uint32_t sym) override {
        switch (sym) {
            case 'c':
            case 'C':
                fill(fBG, 0xFF);
                this->requestDraw();
                return true;
        }
        return false;
    }

    GClick* onFindClickHandler(GPoint loc) override {
        GRect r = GRect::MakeWH(30, 30);
        Brush* b = new RectBrush(r, rand_color(), GBlendMode::kSrc);
        GCanvas* c = GCreateCanvas(fFG).release();
        return new GClick(loc, [this, c, b](GClick* click) {
            if (click->state() == GClick::kUp_State) {
                draw_bitmap(fBG, fFG, GBlendMode::kSrcOver);
                fill(fFG, 0);
                delete c;
                delete b;
            } else {
                b->draw(c, click->curr());
            }
            this->requestDraw();
        });
    }

private:
    void updateTitle() {
        char buffer[100];
        buffer[0] = ' ';
        buffer[1] = 0;
        this->setTitle(buffer);
    }

    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new TestWindow(640, 480);

    return wind->run();
}

