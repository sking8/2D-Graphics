/**
 *  Copyright 2015 Mike Reed
 */

#include "bench.h"
#include "GCanvas.h"
#include "GBitmap.h"
#include "GTime.h"
#include <memory>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static int pixel_diff(GPixel p0, GPixel p1) {
    int da = abs(GPixel_GetA(p0) - GPixel_GetA(p1));
    int dr = abs(GPixel_GetR(p0) - GPixel_GetR(p1));
    int dg = abs(GPixel_GetG(p0) - GPixel_GetG(p1));
    int db = abs(GPixel_GetB(p0) - GPixel_GetB(p1));
    return std::max(da, std::max(dr, std::max(dg, db)));
}

static double compare(const GBitmap& a, const GBitmap& b, int tolerance, bool verbose) {
    GASSERT(a.width() == b.width());
    GASSERT(a.height() == b.height());

    const int total = a.width() * a.height() * 255;

    const GPixel* rowA = a.pixels();
    const GPixel* rowB = b.pixels();

    int total_diff = 0;
    int max_diff = 0;

    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            int diff = pixel_diff(rowA[x], rowB[x]) - tolerance;
            if (diff > 0) {
                total_diff += diff;
                max_diff = std::max(max_diff, diff);
            }
        }
        rowA = (const GPixel*)((const char*)rowA + a.rowBytes());
        rowB = (const GPixel*)((const char*)rowB + b.rowBytes());
    }
    
    double score = 1.0 * (total - total_diff) / total;
    if (verbose) {
        printf("    - score %d, max_diff %d\n", (int)(score * 100), max_diff);
    }
    return score;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static void setup_bitmap(GBitmap* bitmap, int w, int h) {
    size_t rb = w * sizeof(GPixel);
    bitmap->reset(w, h, rb, (GPixel*)calloc(h, rb), GBitmap::kNo_IsOpaque);
}

enum Mode {
    kNormal,
    kForever,
    kOnce,
};

static double handle_proc(GBenchmark* bench, const char path[], GBitmap* bitmap, Mode mode) {
    GISize size = bench->size();
    setup_bitmap(bitmap, size.fWidth, size.fHeight);

    auto canvas = GCreateCanvas(*bitmap);
    if (!canvas) {
        fprintf(stderr, "failed to create canvas for [%d %d] %s\n",
                size.fWidth, size.fHeight, bench->name());
        return 0;
    }

    int N = 100;
    bool forever = false;
    switch (mode) {
        case kNormal: break;
        case kForever: forever = true; break;
        case kOnce: N = 1; break;
    }

    GMSec now = GTime::GetMSec();
    for (int i = 0; i < N || forever; ++i) {
        bench->draw(canvas.get());
    }
    GMSec dur = GTime::GetMSec() - now;
    return dur * 1.0 / N;
}

static bool is_arg(const char arg[], const char name[]) {
    std::string str("--");
    str += name;
    if (!strcmp(arg, str.c_str())) {
        return true;
    }

    char shortVers[3];
    shortVers[0] = '-';
    shortVers[1] = name[0];
    shortVers[2] = 0;
    return !strcmp(arg, shortVers);
}

int main(int argc, char** argv) {
    bool verbose = false;
    Mode mode = kNormal;
    const char* match = NULL;
    const char* report = NULL;
    const char* author = NULL;
    FILE* reportFile = NULL;

    for (int i = 1; i < argc; ++i) {
        if (is_arg(argv[i], "report") && i+2 < argc) {
            report = argv[++i];
            author = argv[++i];
            reportFile = fopen(report, "a");
            if (!reportFile) {
                printf("----- can't open %s for author %s\n", report, author);
                return -1;
            }
        } else if (is_arg(argv[i], "verbose")) {
            verbose = true;
        } else if (is_arg(argv[i], "once")) {
            mode = kOnce;
        } else if (is_arg(argv[i], "match") && i+1 < argc) {
            match = argv[++i];
        } else if (is_arg(argv[i], "forever")) {
            mode = kForever;
        }
    }

    for (int i = 0; gBenchFactories[i]; ++i) {
        std::unique_ptr<GBenchmark> bench(gBenchFactories[i]());
        const char* name = bench->name();
        
        if (match && !strstr(name, match)) {
            continue;
        }
        if (verbose) {
            printf("image: %s\n", name);
        }
        
        GBitmap testBM;
        double dur = handle_proc(bench.get(), name, &testBM, mode);
        printf("bench: %s %g\n", name, dur);

        free(testBM.pixels());
    }
    return 0;
}
