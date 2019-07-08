/*
 Copyright 2015 Mike Reed

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#ifndef GCanvas_DEFINED
#define GCanvas_DEFINED

#include "GMatrix.h"
#include "GPaint.h"
#include "GShader.h"

class GBitmap;
class GPath;
class GPoint;
class GRect;

class GCanvas {
public:
    virtual ~GCanvas() {}

    /**
     *  Add contour(s) to the specified path that will draw a line from p0 to p1 with the specified
     *  width. Note that "width" is the distance from one side of the stroke to the other.
     *
     *  If roundCap is true, the path should also include a circular cap at each end of the line,
     *  where the circle has radius of width/2 and its center is positioned at p0 and p1.
     */
    virtual void final_strokeLine(GPath* dst, GPoint p0, GPoint p1, float width, bool roundCap) {}

    /**
     *  Return a radial-gradient shader.
     *
     *  This is a shader defined by a circle with center point (cx, cy) and a radius.
     *  It supports an array colors (count >= 2) where
     *      color[0]       is the color at the center
     *      color[count-1] is the color at the outer edge of the circle
     *      the other colors (if any) are evenly distributed along the radius
     *
     *  e.g. If there are 4 colors and a radius of 90 ...
     *
     *      color[0] is at the center
     *      color[1] is at a distance of 30 from the center
     *      color[2] is at a distance of 60 from the center
     *      color[3] is at a distance of 90 from the center
     *
     *  Positions outside of the radius are clamped to color[count - 1].
     *  Positions inside the radius are linearly interpolated between the two nearest colors.
     *
     *  Interpolation occurs between GColors, and then it is premultiplied to a GPixel.
     */
    virtual std::unique_ptr<GShader> final_createRadialGradient(GPoint center, float radius,
                                                                const GColor colors[], int count,
                                                                GShader::TileMode mode);
    /**
     *  Save off a copy of the canvas state (CTM), to be later used if the balancing call to
     *  restore() is made. Calls to save/restore can be nested:
     *  save();
     *      save();
     *          concat(...);    // this modifies the CTM
     *          .. draw         // these are drawn with the modified CTM
     *      restore();          // now the CTM is as it was when the 2nd save() call was made
     *      ..
     *  restore();              // now the CTM is as it was when the 1st save() call was made
     */
    virtual void save() = 0;

    /**
     *  Copy the canvas state (CTM) that was record in the correspnding call to save() back into
     *  the canvas. It is an error to call restore() if there has been no previous call to save().
     */
    virtual void restore() = 0;

    /**
     *  Modifies the CTM by preconcatenating the specified matrix with the CTM. The canvas
     *  is constructed with an identity CTM.
     *
     *  CTM' = CTM * matrix
     */
    virtual void concat(const GMatrix& matrix) = 0;

    /**
     *  Fill the entire canvas with the specified color, using the specified blendmode.
     */
    virtual void drawPaint(const GPaint&) = 0;

    /**
     *  Fill the rectangle with the color, using the specified blendmode.
     *
     *  The affected pixels are those whose centers are "contained" inside the rectangle:
     *      e.g. contained == center > min_edge && center <= max_edge
     */
    virtual void drawRect(const GRect&, const GPaint&) = 0;

    /**
     *  Fill the convex polygon with the color and blendmode,
     *  following the same "containment" rule as rectangles.
     */
    virtual void drawConvexPolygon(const GPoint[], int count, const GPaint&) = 0;

    /**
     *  Fill the path with the paint, interpreting the path using winding-fill (non-zero winding).
     */
    virtual void drawPath(const GPath&, const GPaint&) = 0;

    /**
     *  Draw a mesh of triangles, with optional colors and/or texture-coordinates at each vertex.
     *
     *  The triangles are specified by successive triples of indices.
     *      int n = 0;
     *      for (i = 0; i < count; ++i) {
     *          point0 = vertx[indices[n+0]]
     *          point1 = verts[indices[n+1]]
     *          point2 = verts[indices[n+2]]
     *          ...
     *          n += 3
     *      }
     *
     *  If colors is not null, then each vertex has an associated color, to be interpolated
     *  across the triangle. The colors are referenced in the same way as the verts.
     *          color0 = colors[indices[n+0]]
     *          color1 = colors[indices[n+1]]
     *          color2 = colors[indices[n+2]]
     *
     *  If texs is not null, then each vertex has an associated texture coordinate, to be used
     *  to specify a coordinate in the paint's shader's space. If there is no shader on the
     *  paint, then texs[] should be ignored. It is referenced in the same way as verts and colors.
     *          texs0 = texs[indices[n+0]]
     *          texs1 = texs[indices[n+1]]
     *          texs2 = texs[indices[n+2]]
     *
     *  If both colors and texs[] are specified, then at each pixel their values are multiplied
     *  together, component by component.
     */
    virtual void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint& paint) = 0;

    /**
     *  Draw the quad, with optional color and/or texture coordinate at each corner. Tesselate
     *  the quad based on "level":
     *      level == 0 --> 1 quad  -->  2 triangles
     *      level == 1 --> 4 quads -->  8 triangles
     *      level == 2 --> 9 quads --> 18 triangles
     *      ...
     *  The 4 corners of the quad are specified in this order:
     *      top-left --> top-right --> bottom-right --> bottom-left
     *  Each quad is triangulated on the diagonal top-right --> bottom-left
     *      0---1
     *      |  /|
     *      | / |
     *      |/  |
     *      3---2
     *
     *  colors and/or texs can be null. The resulting triangles should be passed to drawMesh(...).
     */
    virtual void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                          int level, const GPaint&) = 0;

    // Helpers

    void translate(float x, float y) {
        this->concat(GMatrix::MakeTranslate(x, y));
    }

    void scale(float x, float y) {
        this->concat(GMatrix::MakeScale(x, y));
    }

    void rotate(float radians) {
        this->concat(GMatrix::MakeRotate(radians));
    }

    void clear(const GColor& color) {
        GPaint paint(color);
        paint.setBlendMode(GBlendMode::kSrc);
        this->drawPaint(paint);
    }

    void fillRect(const GRect& rect, const GColor& color) {
        this->drawRect(rect, GPaint(color));
    }
};

/**
 *  If the bitmap is valid for drawing into, this returns a subclass that can perform the
 *  drawing. If bitmap is invalid, this returns NULL.
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& bitmap);

/**
 *  Implement this, and draw something interesting with polygons, matrices, and shaders.
 *  Dimensions = 512 x 512
 */
void GDrawSomething_polys(GCanvas* canvas);

#endif
