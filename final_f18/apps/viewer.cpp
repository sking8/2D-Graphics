/**
 *  Copyright 2017 Mike Reed
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GRandom.h"
#include "GRect.h"
#include "image.h"

#include "GProxyCanvas.h"

class LimitCanvas : public GProxyCanvas {
    int fIndex, fCount;
public:
    LimitCanvas(GCanvas* proxy, int count) : GProxyCanvas(proxy) {
        fIndex = 0;
        fCount = count; // number to allow
    }

    bool allowDraw() override {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return ++fIndex <= fCount;
    }
};

class CounterCanvas : public GProxyCanvas {
    int fCounter;
public:
    CounterCanvas() : GProxyCanvas(nullptr), fCounter(0) {}
    
    void reset() { fCounter = 0; }
    int getCount() const { return fCounter; }

    bool allowDraw() override {
        fCounter += 1;
        return false;
    }
};

#define SLIDER_W        10
#define SLIDER_MARGIN   20

class ViewerWindow : public GWindow {
    GRect fSlider;
    int fRecIndex = 0;
    int fRecCount;
    int fOpCount = -1;
    float fOpPercent = 1;
    bool fZoomer = false;

public:
    ViewerWindow(int w, int h) : GWindow(w, h) {
        this->onResize(w, h);

        for (fRecCount = 0; gDrawRecs[fRecCount].fDraw; ++fRecCount)
            ;
    }

    static GColor rand_color(GRandom& r) {
        return { 1, r.nextF(), r.nextF(), r.nextF() };
    }

protected:
    void onResize(int w, int h) override {
        fSlider.setLTRB(w - SLIDER_W, SLIDER_MARGIN, w, h - SLIDER_MARGIN);
    }

    void onDraw(GCanvas* canvas) override {
        canvas->fillRect(GRect::MakeXYWH(0, 0, 10000, 10000), {1,1,1,1});

        if (fOpCount < 0) {
            CounterCanvas counter;
            gDrawRecs[fRecIndex].fDraw(&counter);
            fOpCount = counter.getCount();
        }

        canvas->save();
        LimitCanvas limit(canvas, GRoundToInt(fOpPercent * fOpCount));
        gDrawRecs[fRecIndex].fDraw(&limit);
        canvas->restore();

        GRect r = fSlider;
        r.fBottom = r.fTop + fOpPercent * fSlider.height();
        canvas->fillRect(r, { 1, .5, .5, .5 });
        r.fTop = r.fBottom;
        r.fBottom = fSlider.fBottom;
        canvas->fillRect(r, { 1, .75, .75, .75 });
    }

    void onDrawOverlays() override {
        if (fZoomer) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            const int w = this->width();
            const int h = this->height();
            const int r = 3;
            const int d = 2 * r + 1;
            const int z = 16;
            GIRect src = GIRect::MakeXYWH(x - r, y - r, d, d);
            GIRect dst = GIRect::MakeLTRB(w - z * d - SLIDER_W, h - z * d, w - SLIDER_W, h);
            this->drawOverlay(&src, &dst);
        }
    }

    bool onKeyPress(uint32_t sym) override {
        switch (sym) {
            case SDLK_LEFT:
                fRecIndex -= 1;
                if (fRecIndex < 0) {
                    fRecIndex = fRecCount - 1;
                }
                fOpCount = -1;  // signal need to recompute
                fOpPercent = 1;
                this->updateTitle();
                this->requestDraw();
                return true;
            case SDLK_RIGHT:
                fRecIndex += 1;
                if (fRecIndex >= fRecCount) {
                    fRecIndex = 0;
                }
                fOpCount = -1;  // signal need to recompute
                fOpPercent = 1;
                this->updateTitle();
                this->requestDraw();
                return true;
            case 'z':
                fZoomer = !fZoomer;
                return true;
            default:
                break;
        }
        return false;
    }

    GClick* onFindClickHandler(GPoint loc) override {
        if (fSlider.contains(loc.fX, loc.fY)) {
            return new GClick(loc, [this](GClick* click) {
                fOpPercent = GPinToUnit((click->curr().y() - fSlider.top()) / fSlider.height());
                this->requestDraw();
            });
        }
        return nullptr;
    }

private:
    void updateTitle() {
        char buffer[1000];
        sprintf(buffer, "%2d: %s", fRecIndex, gDrawRecs[fRecIndex].fName);
        this->setTitle(buffer);
    }

    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new ViewerWindow(640, 480);

    return wind->run();
}

