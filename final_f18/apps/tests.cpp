/**
 *  Copyright 2015 Mike Reed
 */

#include "tests.h"
#include <string>

static bool is_arg(const char arg[], const char target[]) {
    std::string str("--");
    str += target;
    if (!strcmp(arg, str.c_str())) {
        return true;
    }

    char buffer[3];
    buffer[0] = '-';
    buffer[1] = target[0];
    buffer[2] = 0;
    return !strcmp(arg, buffer);
}

int main(int argc, char** argv) {
    const char* report = NULL;
    const char* author = NULL;
    FILE* reportFile = NULL;
    const char* scoreFile = nullptr;

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
            gTestSuite_Verbose = true;
        } else if (is_arg(argv[i], "scoreFile") && i+1 < argc) {
            scoreFile = argv[++i];
        } else if (is_arg(argv[i], "crash")) {
            gTestSuite_CrashOnFailure = true;
        } else if (is_arg(argv[i], "help")) {
            printf("tests [--verbose][-v] [--crash][-c] [--help][-h]\n");
            printf("--help     show this text\n");
            printf("--crash    crash if a test fails (good when running in a debugger)\n");
            printf("--verbose  give verbose status for each test\n");
            return 0;
        }
    }

    GTestStats stats;

    for (int i = 0; gTestRecs[i].fProc; ++i) {
        GTestStats localStats;
        gTestRecs[i].fProc(&localStats);
        if (gTestSuite_Verbose) {
            printf("%16s: [%3d/%3d]  %g\n", gTestRecs[i].fName,
                   localStats.fPassCounter, localStats.fTestCounter, localStats.percent());
        }
        stats.fTestCounter += localStats.fTestCounter;
        stats.fPassCounter += localStats.fPassCounter;
    }

    const int score = (int)(100*stats.percent());

    printf("%16s: [%3d/%3d]  %d\n", "tests", stats.fPassCounter, stats.fTestCounter, score);
    if (reportFile) {
        fprintf(reportFile, "%s, tests, %d\n", author, score);
    }
    if (scoreFile) {
        FILE* f = fopen(scoreFile, "w");
        if (f) {
            fprintf(f, "%d", score);
            fclose(f);
        } else {
            printf("FAILED TO WRITE TO %s\n", scoreFile);
            return -1;
        }
    }
    return 0;
}
