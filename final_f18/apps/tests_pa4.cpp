/**
 *  Copyright 2018 Mike Reed
 */

#include "GPath.h"
#include "tests.h"

static void test_path(GTestStats* stats) {
    GPoint pts[2];
    GPath path;

    stats->expectTrue(path.bounds() == GRect::MakeWH(0,0), "path0");
    stats->expectTrue(GPath::Iter(path).next(pts) == GPath::kDone, "path1");

    path.moveTo(10, 20);
    stats->expectTrue(path.bounds() == GRect::MakeXYWH(10, 20, 0, 0), "path2");
    path.lineTo(30, 0);
    stats->expectTrue(path.bounds() == GRect::MakeLTRB(10, 0, 30, 20), "path3");

    GPath::Iter iter(path);
    stats->expectTrue(iter.next(pts) == GPath::kMove);
    stats->expectTrue(pts[0] == GPoint{10, 20});
    stats->expectTrue(iter.next(pts) == GPath::kLine);
    stats->expectTrue(pts[0] == GPoint{10, 20});
    stats->expectTrue(pts[1] == GPoint{30, 0});
    stats->expectTrue(iter.next(pts) == GPath::kDone);

    GPath p2 = path;
    stats->expectTrue(p2.bounds() == path.bounds(), "path assign");
    p2.transform(GMatrix::MakeScale(0.5, 2));
    stats->expectTrue(p2.bounds() == GRect::MakeLTRB(5, 0, 15, 40), "path transform");

    const GPoint p[] = { {10, 10}, {20, 20}, {30, 30} };
    int N = 0;
    path.reset();
    path.moveTo(p[0]).lineTo(p[1]).lineTo(p[2]);    N++;
    path.moveTo(p[0]).lineTo(p[1]).lineTo(p[2]);    N++;
    path.moveTo(p[0]).lineTo(p[1]).lineTo(p[2]);    N++;

    GPath::Edger edger(path);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 3; ++j) {
            GASSERT(edger.next(pts) == GPath::kLine);
            GASSERT(pts[0] == p[j]);
            GASSERT(pts[1] == p[(j+1)%3]);
        }
    }
    GASSERT(edger.next(pts) == GPath::kDone);
}

static bool expect_iter(const GPath& path, const GPoint expected_pts[], int count) {
    GPath::Iter iter(path);
    GPoint pts[2];
    if (count > 0) {
        if (iter.next(pts) != GPath::kMove || pts[0] != expected_pts[0]) {
            return false;
        }
        for (int i = 1; i < count; ++i) {
            if (iter.next(pts) != GPath::kLine || pts[1] != expected_pts[i]) {
                return false;
            }
        }
    }
    return iter.next(pts) == GPath::kDone;
}

static void test_path_rect(GTestStats* stats) {
    GPath p;
    stats->expectTrue(GRect::MakeWH(0, 0) == p.bounds(), "path_rect_empty");

    GRect r = GRect::MakeLTRB(10, 20, 30, 40);
    p.addRect(r, GPath::kCW_Direction);
    stats->expectTrue(p.bounds() == r, "path_rect_cw_bounds");
    const GPoint pts0[] = {{10, 20}, {30, 20}, {30, 40}, {10, 40}};
    stats->expectTrue(expect_iter(p, pts0, 4), "path_rect_cw_iter");

    p.reset();
    p.addRect(r, GPath::kCCW_Direction);
    stats->expectTrue(p.bounds() == r, "path_rect_ccw_bounds");
    const GPoint pts1[] = {{10, 20}, {10, 40}, {30, 40}, {30, 20}};
    stats->expectTrue(expect_iter(p, pts1, 4), "path_rect_ccw_iter");
}

static void test_path_poly(GTestStats* stats) {
    const GPoint pts[] = {{-10, -10}, {10, 0}, {0, 10}};
    GPath p;
    p.addPolygon(pts, GARRAY_COUNT(pts));
    stats->expectTrue(GRect::MakeLTRB(-10, -10, 10, 10) == p.bounds(), "path_poly_bounds");
    stats->expectTrue(expect_iter(p, pts, GARRAY_COUNT(pts)), "path_poly_iter");
}

static void test_path_transform(GTestStats* stats) {
    GPath p;
    const GPoint pts[] = {{10, 20}, {30, 40}, {50, 60}, {70, 80}};
    GRect r = GRect::MakeLTRB(10, 20, 70, 80);

    p.addPolygon(pts, GARRAY_COUNT(pts));
    stats->expectTrue(r == p.bounds(), "path_transform_bounds0");

    p.transform(GMatrix::MakeTranslate(-30, 20));
    r.offset(-30, 20);
    stats->expectTrue(r == p.bounds(), "path_transform_bounds1");
}
