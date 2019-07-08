/*
 *  Copyright 2018 Mike Reed
 */

#ifndef GBlendMode_DEFINED
#define GBlendMode_DEFINED

#include "GTypes.h"

enum class GBlendMode {
    kClear,    //!< [0, 0]
    kSrc,      //!< [Sa, Sc]
    kDst,      //!< [Da, Dc]
    kSrcOver,  //!< [Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
    kDstOver,  //!< [Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
    kSrcIn,    //!< [Sa * Da, Sc * Da]
    kDstIn,    //!< [Da * Sa, Dc * Sa]
    kSrcOut,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
    kDstOut,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
    kSrcATop,  //!< [Da, Sc * Da + Dc * (1 - Sa)]
    kDstATop,  //!< [Sa, Dc * Sa + Sc * (1 - Da)]
    kXor,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
};

#endif
