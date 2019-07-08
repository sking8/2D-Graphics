#ifndef GPath_DEFINED
#define GPath_DEFINED

#include <vector>
#include "GPoint.h"
#include "GRect.h"

class GMatrix;

class GPath {
public:
    GPath();
    ~GPath();

    GPath& operator=(const GPath&);

    /**
     *  Erase any previously added points/verbs, restoring the path to its initial empty state.
     */
    GPath& reset();

    /**
     *  Start a new contour at the specified coordinate.
     */
    GPath& moveTo(GPoint);
    GPath& moveTo(float x, float y) { return this->moveTo({x, y}); }

    /**
     *  Connect the previous point (either from a moveTo or lineTo) with a line segment to
     *  the specified coordinate.
     */
    GPath& lineTo(GPoint);
    GPath& lineTo(float x, float y) { return this->lineTo({x, y}); }

    /**
     *  Connect the previous point with a quadratic bezier to the specified coordinates.
     */
    GPath& quadTo(GPoint, GPoint);
    GPath& quadTo(float x0, float y0, float x1, float y1) {
        return this->quadTo({x0, y0}, {x1, y1});
    }

    /**
     *  Connect the previous point with a cubic bezier to the specified coordinates.
     */
    GPath& cubicTo(GPoint, GPoint, GPoint);
    GPath& cubicTo(float x0, float y0, float x1, float y1, float x2, float y2) {
        return this->cubicTo({x0, y0}, {x1, y1}, {x2, y2});
    }

    enum Direction {
        kCW_Direction,
        kCCW_Direction,
    };
    
    /**
     *  Append a new contour, made up of the 4 points of the specified rect, in the specified
     *  direction. The contour will begin at the top-left corner of the rect.
     */
    GPath& addRect(const GRect&, Direction = kCW_Direction);

    /**
     *  Append a new contour with the specified polygon. Calling this is equivalent to calling
     *  moveTo(pts[0]), lineTo(pts[1..count-1]).
     */
    GPath& addPolygon(const GPoint pts[], int count);

    /**
     *  Append a new contour respecting the Direction. The contour should be an approximate
     *  circle (8 quadratic curves will suffice) with the specified center and radius.
     */
    GPath& addCircle(GPoint center, float radius, Direction = kCW_Direction);

    int countPoints() const { return (int)fPts.size(); }

    /**
     *  Return the bounds of all of the control-points in the path.
     *
     *  If there are no points, return {0, 0, 0, 0}
     */
    GRect bounds() const;

    /**
     *  Transform the path in-place by the specified matrix.
     */
    void transform(const GMatrix&);

    enum Verb {
        kMove,  // returns pts[0] from Iter
        kLine,  // returns pts[0]..pts[1] from Iter and Edger
        kQuad,  // returns pts[0]..pts[2] from Iter and Edger
        kCubic, // returns pts[0]..pts[3] from Iter and Edger
        kDone   // returns nothing in pts, Iter/Edger is done
    };

    /**
     *  Walks the path, returning each verb that was entered.
     *  e.g.    moveTo() returns kMove
     *          lineTo() returns kLine
     */
    class Iter {
    public:
        Iter(const GPath&);
        Verb next(GPoint pts[]);

    private:
        const GPoint* fPrevMove;
        const GPoint* fCurrPt;
        const Verb*   fCurrVb;
        const Verb*   fStopVb;
    };

    /**
     *  Walks the path, returning "edges" only. Thus it does not return kMove, but will return
     *  the final closing "edge" for each contour.
     *  e.g.
     *      path.moveTo(A).lineTo(B).lineTo(C).moveTo(D).lineTo(E)
     *  will return
     *      kLine   A..B
     *      kLine   B..C
     *      kLine   C..A
     *      kLine   D..E
     *      kLine   E..D
     *      kDone
     */
    class Edger {
    public:
        Edger(const GPath&);
        Verb next(GPoint pts[]);

    private:
        const GPoint* fPrevMove;
        const GPoint* fCurrPt;
        const Verb*   fCurrVb;
        const Verb*   fStopVb;
        Verb fPrevVerb;
    };

    // maximum number of points returned by Edger::next()
    enum {
        kMaxEdgerPoints = 4
    };

    /**
     *  Given 0 < t < 1, subdivide the src[] quadratic bezier at t into two new quadratics in dst[]
     *  such that
     *  0...t is stored in dst[0..2]
     *  t...1 is stored in dst[2..4]
     */
    static void ChopQuadAt(const GPoint src[3], GPoint dst[5], float t);

    /**
     *  Given 0 < t < 1, subdivide the src[] cubic bezier at t into two new cubics in dst[]
     *  such that
     *  0...t is stored in dst[0..3]
     *  t...1 is stored in dst[3..6]
     */
    static void ChopCubicAt(const GPoint src[4], GPoint dst[7], float t);

private:
    std::vector<GPoint> fPts;
    std::vector<Verb>   fVbs;
};

#endif

