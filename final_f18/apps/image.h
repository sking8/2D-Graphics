/**
 *  Copyright 2015 Mike Reed
 */

#ifndef G_image_DEFINED
#define G_image_DEFINED

class GCanvas;

struct GDrawRec {
    void        (*fDraw)(GCanvas*);
    int         fWidth;
    int         fHeight;
    const char* fName;
    int         fPA;
};

/*
 *  Array is terminated when fDraw is NULL
 */
extern const GDrawRec gDrawRecs[];

#endif
