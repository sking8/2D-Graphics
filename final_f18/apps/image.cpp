/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GBitmap.h"
#include <string>

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

    int total = 0;

    const GPixel* rowA = a.pixels();
    const GPixel* rowB = b.pixels();

    int total_diff = 0;
    int max_diff = 0;

    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            // we don't score transparent pixels if both a and b are transparent (background)
            if (!rowA[x] && !rowB[x]) {
                continue;
            }

            int diff = pixel_diff(rowA[x], rowB[x]) - tolerance;
            if (diff > 0) {
                total_diff += diff;
                max_diff = std::max(max_diff, diff);
            }
            total += 255;
        }
        rowA = (const GPixel*)((const char*)rowA + a.rowBytes());
        rowB = (const GPixel*)((const char*)rowB + b.rowBytes());
    }

    double score = 1.0 * (total - total_diff) / total;
    GASSERT(score >= 0 && score <= 1);
    score *= score;
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

static void handle_proc(const GDrawRec& rec, const char path[], GBitmap* bitmap) {
    setup_bitmap(bitmap, rec.fWidth, rec.fHeight);

    auto canvas = GCreateCanvas(*bitmap);
    if (!canvas) {
        fprintf(stderr, "failed to create canvas for [%d %d] %s\n",
                rec.fWidth, rec.fHeight, rec.fName);
        return;
    }

    canvas->clear({0, 0, 0, 0});
    rec.fDraw(canvas.get());

    if (!bitmap->writeToFile(path)) {
        fprintf(stderr, "failed to write %s\n", path);
    }
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

static void add_image(FILE* f, const char path[], const char name[], const char suffix[],
                      const GBitmap& bm) {
    std::string str(name);
    str += "__";
    str += suffix;
    str += ".png";
    fprintf(f, "<a href=\"%s\"><img src=\"%s\" /></a>\n", str.c_str(), str.c_str());

    std::string full(path);
    full += "/";
    full += str;
    bm.writeToFile(full.c_str());
}

static void add_diff_to_file(FILE* f, const GBitmap& test, const GBitmap& orig, const char path[],
                             const char name[]) {
    const int w = test.width();
    const int h = test.height();
    GBitmap diff0, diff1;
    setup_bitmap(&diff0, w, h);
    setup_bitmap(&diff1, w, h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int diff = pixel_diff(*test.getAddr(x, y), *orig.getAddr(x, y));
            *diff0.getAddr(x, y) = GPixel_PackARGB(0xFF, diff, diff, diff);
            if (diff > 0) {
                diff = 0xFF;
            }
            *diff1.getAddr(x, y) = GPixel_PackARGB(0xFF, diff, diff, diff);
        }
    }

    fprintf(f, "%s<br/>\n", name);
    add_image(f, path, name, "test", test); fprintf(f, "&nbsp;&nbsp;");
    add_image(f, path, name, "orig", orig); fprintf(f, "&nbsp;&nbsp;");
    add_image(f, path, name, "dif0", diff0); fprintf(f, "&nbsp;&nbsp;");
    add_image(f, path, name, "dif1", diff1); fprintf(f, "<br><br>\n");
}

static int gPACounts[10] = { 0,0,0,0,0,0,0,0,0,0 };
static int gDrawCount;

int main(int argc, char** argv) {
    bool verbose = false;
    std::string root;
    const char* match = NULL;
    const char* expected = NULL;
    const char* diffDir = NULL;
    const char* report = NULL;
    const char* author = NULL;
    const char* scoreFile = nullptr;
    FILE* reportFile = NULL;
    FILE* diffFile = NULL;
    int tolerance = 0;
    int targetPA = -1;
    int oneShot = -1;

    for (int i = 0; gDrawRecs[i].fDraw; ++i) {
        GASSERT((unsigned)gDrawRecs[i].fPA < GARRAY_COUNT(gPACounts));
        gPACounts[gDrawRecs[i].fPA] += 1;
        if (gDrawRecs[i].fPA > 0) {
            gDrawCount += 1;
        }
    }

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
        } else if (is_arg(argv[i], "oneshot") && i+1 < argc) {
            oneShot = atoi(argv[++i]);
            GASSERT(oneShot >= 0 && oneShot < gDrawCount);
        } else if (is_arg(argv[i], "write") && i+1 < argc) {
            root = argv[++i];
        } else if (is_arg(argv[i], "match") && i+1 < argc) {
            match = argv[++i];
        } else if (is_arg(argv[i], "expected") && i+1 < argc) {
            expected = argv[++i];
        } else if (is_arg(argv[i], "pa") && i+1 < argc) {
            targetPA = atoi(argv[++i]);
        } else if (is_arg(argv[i], "tolerance") && i+1 < argc) {
            tolerance = atoi(argv[++i]);
            GASSERT(tolerance >= 0);
        } else if (is_arg(argv[i], "scoreFile") && i+1 < argc) {
            scoreFile = argv[++i];
        } else if (is_arg(argv[i], "diff") && i+1 < argc) {
            diffDir = argv[++i];
            std::string path(diffDir);
            path += "/index.html";
            diffFile = fopen(path.c_str(), "w");
            if (!diffFile) {
                printf("------- failed to create %s\n", path.c_str());
            } else {
                fprintf(diffFile, "<h3>Test Orig Diff DIFF</h3>\n");
            }
        }
    }

    if (root.size() > 0 && root[root.size() - 1] != '/') {
        root += "/";
        if (!mk_dir(root.c_str())) {
            return -1;
        }
    }

    if (verbose) {
        printf("--write %s\n", root.c_str());
        printf("--match %s\n", match);
        printf("--expected %s\n", expected);
        printf("--tolerance %d\n", tolerance);
        if (scoreFile) {
            printf("--scoreFile %s\n", scoreFile);
        }
    }

    double max_score = (1 << gDrawRecs[gDrawCount - 1].fPA) - 1;

    double percent_correct = 0;
    double counter = 0;
    for (int i = 0; gDrawRecs[i].fDraw; ++i) {
        if (targetPA > 0 && targetPA != gDrawRecs[i].fPA) {
            continue;
        }
        std::string path(root);
        path += gDrawRecs[i].fName;
        path += ".png";

        const bool score_me = gDrawRecs[i].fPA > 0;

        double weight = 0;
        if (score_me) {
            weight = 1 << (gDrawRecs[i].fPA - 1);
            weight /= gPACounts[gDrawRecs[i].fPA];
            counter += weight;
        }

        if (match && !strstr(path.c_str(), match)) {
            continue;
        }
        if (oneShot >= 0 && oneShot != i) {
            continue;
        }

        if (verbose && score_me) {
            printf("image: index=%2d pa=%d %s\n", i, gDrawRecs[i].fPA, path.c_str());
        }
        
        GBitmap testBM;
        handle_proc(gDrawRecs[i], path.c_str(), &testBM);

        if (expected && score_me) {
            std::string exp_path(expected);
            exp_path += "/";
            exp_path += gDrawRecs[i].fName;
            exp_path += ".png";
            GBitmap expectedBM;
    
            if (!expectedBM.readFromFile(exp_path.c_str())) {
                printf("- failed to load <%s>\n", exp_path.c_str());
            } else {
                double correct = compare(testBM, expectedBM, tolerance, verbose);
                if (correct < 1 && diffFile != NULL) {
                    add_diff_to_file(diffFile, testBM, expectedBM, diffDir, gDrawRecs[i].fName);
                }
                double individual_score = correct * weight;

                percent_correct += individual_score;

                if (oneShot == i) {
                    individual_score = individual_score * 100 / max_score;
                    if (verbose) {
                        printf("[%d] score=%g %s\n", i, individual_score, path.c_str());
                    }
                    if (scoreFile) {
                        FILE* f = fopen(scoreFile, "w");
                        if (f) {
                            fprintf(f, "%g\n", individual_score);
                            fclose(f);
                            return 0;
                        } else {
                            printf("FAILED TO WRITE TO %s\n", scoreFile);
                            return -1;
                        }
                    }
                }
            }
        }
        
        free(testBM.pixels());
    }
    if (diffFile) {
        fclose(diffFile);
    }

    GASSERT(targetPA > 0 || floor(counter + 0.5) == max_score);

    int image_score = (int)(percent_correct * 100 / counter);
    if (expected && (oneShot < 0)) {
        printf("           image: %d\n", image_score);
    }
    if (reportFile) {
        fprintf(reportFile, "%s, image, %d\n", author, image_score);
    }
    if (scoreFile) {
        FILE* f = fopen(scoreFile, "w");
        if (f) {
            fprintf(f, "%d", image_score);
            fclose(f);
        } else {
            printf("FAILED TO WRITE TO %s\n", scoreFile);
            return -1;
        }
    }
    return 0;
}
