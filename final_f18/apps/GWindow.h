/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GWindow_DEFINED
#define GWindow_DEFINED

#include <SDL2/SDL.h>
#include <functional>

#include "GBitmap.h"
#include "GPoint.h"

class GCanvas;
class GClick;
class GIRect;

class GWindow {
public:
    int run();

    void requestDraw();

protected:
    GWindow(int initial_width, int initial_height);
    virtual ~GWindow();

    virtual void onUpdate(const GBitmap&, GCanvas*);
    virtual void onDraw(GCanvas*) {}
    virtual void onResize(int w, int h) {}
    virtual bool onKeyPress(uint32_t) { return false; }
    virtual GClick* onFindClickHandler(GPoint) { return NULL; }
    virtual void onDrawOverlays() {}

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    
    void setTitle(const char title[]);
    void drawOverlay(const GIRect* src, const GIRect* dst);


private:
    GClick*     fClick;
    
    GBitmap fBitmap;
    std::unique_ptr<GCanvas> fCanvas;
    int fWidth;
    int fHeight;
    bool fNeedDraw;

    SDL_Window*   fWindow;
    SDL_Renderer* fRenderer;
    SDL_Texture*  fTexture;

    uint32_t fInvalEventType;

    bool handleEvent(const SDL_Event&);
    void setupBitmap(int w, int h);
    void pushEvent(int code) const;
};

class GClick {
public:
    GClick(GPoint, std::function<void(GClick*)>);
    
    enum State {
        kDown_State,
        kMove_State,
        kUp_State
    };
    
    State state() const { return fState; }
    GPoint curr() const { return fCurr; }
    GPoint prev() const { return fPrev; }
    GPoint orig() const { return fOrig; }

    void callback() { fFunc(this); }

private:
    GPoint  fCurr, fPrev, fOrig;
    State   fState;
    std::function<void(GClick*)> fFunc;

    friend class GWindow;
};

#endif
