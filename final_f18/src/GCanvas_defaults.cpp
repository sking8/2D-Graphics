/*
 *  Copyright 2018 Mike Reed
 */

#include "GCanvas.h"
#include "GShader.h"

std::unique_ptr<GShader> GCanvas::final_createRadialGradient(GPoint center, float radius,
                                                             const GColor colors[], int count,
                                                             GShader::TileMode) {
    return nullptr;
}
