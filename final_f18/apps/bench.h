/**
 *  Copyright 2016 Mike Reed
 *
 *  590 Introduction to Graphics
 */

#ifndef CS590_bench_DEFINED
#define CS590_bench_DEFINED

#include "GPoint.h"

class GCanvas;

class GBenchmark {
public:
    virtual ~GBenchmark() {}

    virtual const char* name() const = 0;
    virtual GISize size() const = 0;
    virtual void draw(GCanvas*) = 0;

    typedef GBenchmark* (*Factory)();
};

/*
 *  Array is terminated with nullptr
 */
extern const GBenchmark::Factory gBenchFactories[];

#endif
