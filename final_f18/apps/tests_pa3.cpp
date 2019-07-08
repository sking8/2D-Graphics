/**
 *  Copyright 2018 Mike Reed
 */

#include "GMatrix.h"
#include "tests.h"

static bool ie_eq(float a, float b) {
    return fabs(a - b) <= 0.00001f;
}

static bool is_eq(const GMatrix& m, float a, float b, float c, float d, float e, float f) {
    //    printf("%g %g %g    %g %g %g\n", m[0], m[1], m[2], m[3], m[4], m[5]);
    return ie_eq(m[0], a) && ie_eq(m[1], b) && ie_eq(m[2], c) &&
    ie_eq(m[3], d) && ie_eq(m[4], e) && ie_eq(m[5], f);
}

static void test_matrix(GTestStats* stats) {
    GMatrix m;
    stats->expectTrue(is_eq(m, 1, 0, 0, 0, 1, 0), "matrix_identity");
    m.setTranslate(2.5, -4);
    stats->expectTrue(is_eq(m, 1, 0, 2.5, 0, 1, -4), "matrix_setTranslate");
    m.setScale(2.5, -4);
    stats->expectTrue(is_eq(m, 2.5, 0, 0, 0, -4, 0), "matrix_setScale");
    m.setRotate(M_PI);
    stats->expectTrue(is_eq(m, -1, 0, 0, 0, -1, 0), "matrix_setRotate");

    GMatrix m2, m3;
    m.setScale(2, 3);
    m2.setScale(-1, -2);
    m3.setConcat(m, m2);
    stats->expectTrue(is_eq(m3, -2, 0, 0, 0, -6, 0), "matrix_setConcat0");

    m2.setTranslate(5, 6);
    m3.setConcat(m2, m);
    stats->expectTrue(is_eq(m3, 2, 0, 5, 0, 3, 6), "matrix_setConcat1");

    // make sure setConcat handles aliasing (dst == secundo)
    m2.setConcat(m2, m);
    stats->expectTrue(is_eq(m2, 2, 0, 5, 0, 3, 6), "matrix_setConcat2");
}

static void test_matrix_inv(GTestStats* stats) {
    GMatrix m, m2, m3;

    stats->expectTrue(m.invert(&m2) && m == m2, "matrix_invert0");
    m.setScale(0.5f, 4);
    stats->expectTrue(m.invert(&m2) && is_eq(m2, 2, 0, 0, 0, 0.25f, 0), "matrix_invert1");
    // check for alias detection of src and dst
    stats->expectTrue(m.invert(&m)  && is_eq(m,  2, 0, 0, 0, 0.25f, 0), "matrix_invert2");

    m.setRotate(M_PI/3);
    m.postTranslate(3, 4);
    stats->expectTrue(m.invert(&m2), "matrix_invert3");
    m3.setConcat(m, m2);
    stats->expectTrue(is_eq(m3,  1, 0, 0, 0, 1, 0), "matrix_invert4");
}

static void test_matrix_map(GTestStats* stats) {
    const GPoint src[] = { {0, 0}, {1, 1}, {-3, 4}, {0.5f, -0.125} };
    GPoint dst[4], dst2[4];

    GMatrix m;
    m.setScale(2, 2);
    m.postTranslate(3, 4);
    m.mapPoints(dst, src, 4);
    const GPoint expected[] = { {3, 4}, {5, 6}, {-3, 12}, {4, 3.75f} };
    bool equal = true;
    for (int i = 0; i < 4; ++i) {
        equal &= dst[i].x() == expected[i].x();
        equal &= dst[i].y() == expected[i].y();
    }
    stats->expectTrue(equal, "matrix_mappts0");

    m.setRotate(M_PI/3);
    m.postTranslate(3, 4);
    m.mapPoints(dst, src, 4);
    memcpy(dst2, src, sizeof(src));
    m.mapPoints(dst2, dst2, 4);
    stats->expectTrue(memcmp(dst, dst2, sizeof(src)) == 0, "matrix_mappts1");
}


