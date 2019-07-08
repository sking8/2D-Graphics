/**
 *  Copyright 2017 Mike Reed
 */

#include <vector>

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GRandom.h"
#include "GRect.h"
#include "image.h"

static float bounce(float value, float min, float max, float* dir) {
    if (value < min) {
        value = min;
        *dir = - *dir;
    } else if (value > max) {
        value = max;
        *dir = - *dir;
    }
    return value;
}

static GRandom gRand;

struct Shape {
    enum {
        N = 10
    };
    GPoint  fPos;
    GVector fVec;

    float   fWidth, fHeight;
    GColor  fColor;

    GPoint  fPts[N];
    int     fCount;

    Shape(GPoint pos, GVector vec, float w, float h, GColor c) {
        fPos = pos;
        fVec = vec;
        fWidth = w;
        fHeight = h;
        fColor = c;

        int n = 3 + gRand.nextU() % (N - 3);

        fCount = n;
        for (int i = 0; i < n; ++i) {
            fPts[i] = {
                pos.fX + 0.6f * w * sinf(i * 2 * M_PI / n),
                pos.fY + 0.6f * w * cosf(i * 2 * M_PI / n),
            };
        }
    }

    GRect rect() const {
        return GRect::MakeXYWH(fPos.fX - fWidth*0.5, fPos.fY - fHeight*0.5, fWidth, fHeight);
    }

    void bounce(const GRect& r, float speed) {
        float nx = ::bounce(fPos.fX + fVec.fX * speed, r.left(), r.right(), &fVec.fX);
        float ny = ::bounce(fPos.fY + fVec.fY * speed, r.top(), r.bottom(), &fVec.fY);

        float dx = nx - fPos.fX;
        float dy = ny - fPos.fY;
        for (int i = 0; i < fCount; ++i) {
            fPts[i].fX += dx;
            fPts[i].fY += dy;
        }
        fPos.fX = nx;
        fPos.fY = ny;
    }

    void draw(GCanvas* canvas) const {
        canvas->drawConvexPolygon(fPts, fCount, GPaint(fColor));
    }
};

class ViewerWindow : public GWindow {
    std::vector<Shape>  fShapes;
    GRandom             fRand;
    float               fSpeed = 0.01f;
    bool                fDoAnim = true;

    GPoint              fArrow_start,
                        fArrow_stop;
    bool                fArrow = false;

public:
    ViewerWindow(int w, int h) : GWindow(w, h) {
        fShapes.push_back({ {w*0.5f, h*0.5f}, {w/30.f, h/20.f}, 30, 30, { 1, 0, 0, 0 }});
    }

    static GColor rand_color(GRandom& r) {
        return { 1, r.nextF(), r.nextF(), r.nextF() };
    }

    static void draw_line(GCanvas* canvas, GPoint a, GPoint b) {
        const int n = 6;
        GVector v = (b - a) * (1.0f / n);
        const float r = 3;
        const float d = 2*r;
        for (int i = 0; i <= n; ++i) {
            canvas->fillRect(GRect::MakeXYWH(a.fX - r, a.fY - r, d, d), { 0.5, 0, 0, 0 });
            a += v;
        }
    }

    static bool contains(const GRect& r, const GPoint& p) {
        return r.left() < p.x() && r.right() >= p.x() &&
               r.top() < p.y() && r.bottom() >= p.y();
    }
protected:
    void onDraw(GCanvas* canvas) override {
        for (int i = fShapes.size() - 1; i > 0; --i) {
            if (contains(fShapes[0].rect(), fShapes[i].fPos)) {
                fShapes[0].fColor = fShapes[i].fColor;
                fShapes[0].fWidth += 1;
                fShapes[0].fHeight += 1;
                fShapes.erase(fShapes.begin() + i);
                this->updateTitle();
            }
        }

        const GRect r = GRect::MakeWH(this->width(), this->height());
        canvas->fillRect(r, {1,1,1,1});
        for (auto& s : fShapes) {
            if (fDoAnim) {
                s.bounce(r, fSpeed);
            }
            s.draw(canvas);
        }

        if (fArrow) {
            draw_line(canvas, fArrow_start, fArrow_stop);
        }
        if (fDoAnim) {
            this->requestDraw();
        }
    }

    bool onKeyPress(uint32_t sym) override {
        return false;
    }

    GClick* onFindClickHandler(GPoint loc) override {
        fDoAnim = false;
        fShapes.push_back({ loc, {0,0}, 20, 20, rand_color(fRand) });
        int index = fShapes.size() - 1;
        return new GClick(loc, [this, index](GClick* click) {
            fArrow = true;
            fArrow_start = click->orig();
            fArrow_stop  = click->curr();
            this->requestDraw();

            if (click->state() == GClick::kUp_State) {
                fShapes[index].fVec = click->orig() - click->curr();
                const float min = 8;
                if (fShapes[index].fVec.length() < min) {
                    fShapes[index].fVec.set(sinf(click->orig().x())*min,
                                            cosf(click->orig().x())*min);
                }
                fDoAnim = true;
                fArrow = false;
                this->updateTitle();
            }
        });
    }

private:
    void updateTitle() {
        char buffer[100];
        sprintf(buffer, "%lu", fShapes.size() - 1);
        this->setTitle(buffer);
    }

    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    return ViewerWindow(640, 480).run();
}

