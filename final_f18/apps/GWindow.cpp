/**
 *  Copyright 2015 Mike Reed
 */

#include "GWindow.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GRect.h"
#include "GTime.h"
#include <stdio.h>

GClick::GClick(GPoint loc, std::function<void(GClick*)> func) : fFunc(func) {
    fCurr = fPrev = fOrig = loc;
    fState = kDown_State;
}

GWindow::GWindow(int width, int height) {
    fClick = NULL;
    fWidth = width;
    fHeight = height;

    this->setupBitmap(width, height);
    fCanvas = GCreateCanvas(fBitmap);

    uint32_t flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    fWindow = SDL_CreateWindow("An SDL2 window",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               width, height, flags);
    if (!fWindow) {
        printf("Can't create window: %s\n", SDL_GetError());
        return;
    }

    fRenderer = SDL_CreateRenderer(fWindow, -1, 0);

    fTexture = SDL_CreateTexture(fRenderer, SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_STREAMING,
                                            width, height);

    fInvalEventType = SDL_RegisterEvents(1);
}

GWindow::~GWindow() {
    free(fBitmap.pixels());
}

void GWindow::setTitle(const char title[]) {
    SDL_SetWindowTitle(fWindow, title);
}

void GWindow::pushEvent(int code) const {
    SDL_Event u;
    u.type = fInvalEventType;
    u.user.code = code;
    u.user.data1 = nullptr;
    u.user.data2 = nullptr;
    SDL_PushEvent(&u);
}

void GWindow::requestDraw() {
    if (!fNeedDraw) {
        fNeedDraw = true;
        this->pushEvent(42);
    }
}

bool GWindow::handleEvent(const SDL_Event& evt) {
//     printf("event %d\n", evt->type);
    switch (evt.type) {
        case SDL_WINDOWEVENT:
            switch (evt.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    fWidth = evt.window.data1;
                    fHeight = evt.window.data2;
                    this->onResize(fWidth, fHeight);

                    SDL_DestroyTexture(fTexture);
                    fTexture = SDL_CreateTexture(fRenderer,
                                                 SDL_PIXELFORMAT_ARGB8888,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 fWidth, fHeight);

                    this->setupBitmap(fWidth, fHeight);
                    fCanvas = GCreateCanvas(fBitmap);
                    fNeedDraw = true;
                    return true;
            }
            break;
        case SDL_KEYDOWN: {
            unsigned sym = evt.key.keysym.sym;
            if (evt.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
                if (sym >= 'a' && sym <= 'z') {
                    sym += 'A' - 'a';
                }
            }
            if (this->onKeyPress(sym)) {
                return true;
            }
        } break;
        case SDL_MOUSEBUTTONDOWN:
            // seem to get wacky down events when entering a window on mac, but only when
            // .which is non-zero
            if (evt.button.which) {
                break;
            }
            if (fClick) {
                delete fClick;
            }
            fClick = this->onFindClickHandler(GPoint::Make(evt.button.x, evt.button.y));
            if (fClick) {
                return true;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (fClick) {
                fClick->fState = GClick::kUp_State;
                fClick->callback();
                delete fClick;
                fClick = nullptr;
                return true;
            }
            break;
        case SDL_MOUSEMOTION:
            if (fClick) {
                fClick->fState = GClick::kMove_State;
                fClick->fPrev = fClick->fCurr;
                fClick->fCurr.set(evt.motion.x, evt.motion.y);
                fClick->callback();
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

void GWindow::setupBitmap(int w, int h) {
    if (fBitmap.pixels()) {
        free(fBitmap.pixels());
    }
    size_t rb = w * sizeof(GPixel);
    fBitmap.reset(w, h, rb, (GPixel*)calloc(h, rb), GBitmap::kNo_IsOpaque);
}

static SDL_Rect make(const GIRect& r) {
    return { r.x(), r.y(), r.width(), r.height() };
}

static SDL_Rect make(int w, int h) {
    return { 0, 0, w, h };
}

void GWindow::drawOverlay(const GIRect* src, const GIRect* dst) {
    SDL_Rect s = src ? make(*src) : make(fWidth, fHeight);
    SDL_Rect d = dst ? make(*dst) : make(fWidth, fHeight);
    SDL_RenderCopy(fRenderer, fTexture, &s, &d);
}

void GWindow::onUpdate(const GBitmap& bitmap, GCanvas* canvas) {
    this->onDraw(canvas);
}

int GWindow::run() {
    if (!fWindow) {
        return -1;
    }

    this->requestDraw();

    SDL_Event e;
    while (SDL_WaitEvent(&e) && e.type != SDL_QUIT) {
        this->handleEvent(e);

        if (fNeedDraw) {
            fNeedDraw = false;  // clear this before we call onDraw
            this->onUpdate(fBitmap, fCanvas.get());
            SDL_UpdateTexture(fTexture, nullptr, fBitmap.pixels(), fBitmap.rowBytes());
        }
        SDL_RenderCopy(fRenderer, fTexture, nullptr, nullptr);
        this->onDrawOverlays();

        SDL_RenderPresent(fRenderer);
    }
    return 0;
}
