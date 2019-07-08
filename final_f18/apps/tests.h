/**
 *  Copyright 2015 Mike Reed
 */

#ifndef GTestStats_DEFINED
#define GTestStats_DEFINED

#include "GTypes.h"

extern bool gTestSuite_Verbose;
extern bool gTestSuite_CrashOnFailure;

struct GTestStats {
    GTestStats() : fTestCounter(0), fPassCounter(0) {}

    template <typename T> void expectEQ(T a, T b, const char* msg = nullptr) {
        this->expectTrue(a == b, msg);
    }
    
    template <typename T> void expectNE(T a, T b, const char* msg = nullptr) {
        this->expectTrue(a != b, msg);
    }

    void expectNULL(const void* ptr, const char* msg = nullptr) {
        this->expectTrue(NULL == ptr, msg);
    }
    
    void expectPtr(const void* ptr, const char* msg = nullptr) {
        this->expectTrue(NULL != ptr, msg);
    }

    void expectFalse(bool pred, const char* msg = nullptr) {
        this->expectTrue(!pred, msg);
    }
    
    void expectTrue(bool pred, const char* msg = nullptr) {
        this->didTest(pred, msg);
        fPassCounter += (int)pred != 0;
        fTestCounter += 1;
    }

    float percent() const {
        if (fTestCounter) {
            return 1.0f * fPassCounter / fTestCounter;
        } else {
            return 0;
        }
    }
    
    int fTestCounter;
    int fPassCounter;

private:
    void didTest(bool success, const char* msg) {
        if (gTestSuite_Verbose || !success) {
            printf("tests: %s: %s\n", msg, success ? "passed" : "failed");
        }
        if (gTestSuite_CrashOnFailure) {
            GASSERT(false);
        }
    }
};

struct GTestRec {
    void (*fProc)(GTestStats*);
    const char* fName;
};

/*
 *  Array is terminated when fProc is NULL
 */
extern const GTestRec gTestRecs[];

#endif
