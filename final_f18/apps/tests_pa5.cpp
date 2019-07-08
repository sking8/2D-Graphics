/**
 *  Copyright 2018 Mike Reed
 */

#include "GPath.h"
#include "tests.h"

static void test_edger_quads(GTestStats* stats) {
    const GPoint p[] = { {10, 10}, {20, 20}, {30, 30} };
    int N = 0;

    GPath path;
    path.moveTo(p[0]).quadTo(p[1], p[2]);  N++;
    path.moveTo(p[0]).quadTo(p[1], p[2]);  N++;
    path.moveTo(p[0]).quadTo(p[1], p[2]);  N++;

    GPath::Edger edger(path);
    GPoint pts[3];
    for (int i = 0; i < N; ++i) {
        GASSERT(edger.next(pts) == GPath::kQuad);
        for (int j = 0; j < 3; ++j) {
            GASSERT(pts[j] == p[j]);
        }
    }
    GASSERT(edger.next(pts) == GPath::kLine);
    GASSERT(pts[0] == p[2]);
    GASSERT(pts[1] == p[0]);
    GASSERT(edger.next(pts) == GPath::kDone);
}

static void test_path_circle(GTestStats* stats) {
    GPath p;
    
    for (GPath::Direction dir : { GPath::kCW_Direction, GPath::kCCW_Direction }) {
        p.reset();
        p.addCircle({10, 10}, 10, dir);
        stats->expectTrue(GRect::MakeWH(20, 20) == p.bounds(), "path_circle_bounds");
    }
}

