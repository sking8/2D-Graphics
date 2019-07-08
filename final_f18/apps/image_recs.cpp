/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"

#include "image_final.cpp"

const GDrawRec gDrawRecs[] = {

    { final_radial,     512, 512,   "final_radial", 1 },
    { final_stroke,     512, 512,   "final_stroke", 1 },

    { nullptr, 0, 0, nullptr },
};
