/*
 *  Copyright 2018 Mike Reed
 */

#include "GPath.h"
#include "GMatrix.h"

GPath::GPath() {}
GPath::~GPath() {}

GPath& GPath::operator=(const GPath& src) {
    if (this != &src) {
        fPts = src.fPts;
        fVbs = src.fVbs;
    }
    return *this;
}

GPath& GPath::reset() {
    fPts.clear();
    fVbs.clear();
    return *this;
}

GPath& GPath::moveTo(GPoint p) {
    fPts.push_back(p);
    fVbs.push_back(kMove);
    return *this;
}

GPath& GPath::lineTo(GPoint p) {
    GASSERT(fVbs.size() > 0);
    fPts.push_back(p);
    fVbs.push_back(kLine);
    return *this;
}

GPath& GPath::quadTo(GPoint p1, GPoint p2) {
    GASSERT(fVbs.size() > 0);
    fPts.push_back(p1);
    fPts.push_back(p2);
    fVbs.push_back(kQuad);
    return *this;
}

GPath& GPath::cubicTo(GPoint p1, GPoint p2, GPoint p3) {
    GASSERT(fVbs.size() > 0);
    fPts.push_back(p1);
    fPts.push_back(p2);
    fPts.push_back(p3);
    fVbs.push_back(kCubic);
    return *this;
}

/////////////////////////////////////////////////////////////////

GPath::Iter::Iter(const GPath& path) {
    fPrevMove = nullptr;
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
}

GPath::Verb GPath::Iter::next(GPoint pts[]) {
    GASSERT(fCurrVb <= fStopVb);
    if (fCurrVb == fStopVb) {
        return kDone;
    }
    Verb v = *fCurrVb++;
    switch (v) {
        case kMove:
            fPrevMove = fCurrPt;
            pts[0] = *fCurrPt++;
            break;
        case kLine:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            break;
        case kQuad:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            pts[2] = *fCurrPt++;
            break;
        case kCubic:
            pts[0] = fCurrPt[-1];
            pts[1] = *fCurrPt++;
            pts[2] = *fCurrPt++;
            pts[3] = *fCurrPt++;
            break;
#if 0
        case kClose:
            pts[0] = fCurrPt[-1];
            pts[1] = *fPrevMove;
            break;
#endif
        case kDone:
            GASSERT(false); // not reached
    }
    return v;
}

GPath::Edger::Edger(const GPath& path) {
    fPrevMove = nullptr;
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
    fPrevVerb = kDone;
}

GPath::Verb GPath::Edger::next(GPoint pts[]) {
    GASSERT(fCurrVb <= fStopVb);
    bool do_return = false;
    while (fCurrVb < fStopVb) {
        switch (*fCurrVb++) {
            case kMove:
                if (fPrevVerb == kLine) {
                    pts[0] = fCurrPt[-1];
                    pts[1] = *fPrevMove;
                    do_return = true;
                }
                fPrevMove = fCurrPt++;
                fPrevVerb = kMove;
                break;
            case kLine:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                fPrevVerb = kLine;
                return kLine;
            case kQuad:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                pts[2] = *fCurrPt++;
                fPrevVerb = kQuad;
                return kQuad;
            case kCubic:
                pts[0] = fCurrPt[-1];
                pts[1] = *fCurrPt++;
                pts[2] = *fCurrPt++;
                pts[3] = *fCurrPt++;
                fPrevVerb = kCubic;
                return kCubic;
#if 0
            case kClose:
                if (fPrevVerb == kLine) {
                    pts[0] = fCurrPt[-1];
                    pts[1] = *fPrevMove;
                    do_return = true;
                }
                fPrevVerb = kClose;
                break;
#endif
            default:
                GASSERT(false); // not reached
        }
        if (do_return) {
            return kLine;
        }
    }
    if (fPrevVerb >= kLine && fPrevVerb <= kCubic) {
        pts[0] = fCurrPt[-1];
        pts[1] = *fPrevMove;
        fPrevVerb = kDone;
        return kLine;
    } else {
        return kDone;
    }
}
