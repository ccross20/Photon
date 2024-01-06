#ifndef MATHUTILS_H
#define MATHUTILS_H
#include "data/types.h"

namespace photon {

const double intersection_epsilon = 1.0e-30;

//--------------------------------------------------------calc_sq_distance
double calc_sq_distance(double x1, double y1, double x2, double y2)
{
    double dx = x2-x1;
    double dy = y2-y1;
    return dx * dx + dy * dy;
}

//--------------------------------------------------------calc_sq_distance
double calc_sq_distance(point_d pt1, point_d pt2)
{
    double dx = pt2.x-pt1.x;
    double dy = pt2.y-pt1.y;
    return dx * dx + dy * dy;
}


double calc_bezier(double start, double c1, double c2, double end, double t)
{
    double t_ = (1.0 - t);
    double tt_ = t_ * t_;
    double ttt_ = t_ * t_ * t_;
    double tt = t * t;
    double ttt = t * t * t;

    return start * ttt_
           + 3.0 *  c1 * tt_ * t
           + 3.0 *  c2 * t_ * tt
           + end * ttt;
}

point_d calc_bezier(point_d start, point_d c1, point_d c2, point_d end, double t)
{
    return point_d{calc_bezier(start.x,c1.x,c2.x,end.x,t),calc_bezier(start.y,c1.y,c2.y,end.y,t)};
}


point_d calc_bezier_closest_point(point_d start, point_d c1, point_d c2, point_d end, point_d target, double *t = nullptr, uint iterations = 12)
{
    point_d winPt;

    point_d ptA;
    point_d ptB;

    double center = .5;
    double length = .5;
    for(uint i = 0; i < iterations; ++i)
    {
        length *= .5;

        ptA = calc_bezier(start, c1, c2, end, center + length);
        ptB = calc_bezier(start, c1, c2, end, center - length);

        double testPtA = calc_sq_distance(ptA.x, ptA.y, target.x, target.y);
        double testPtB = calc_sq_distance(ptB.x, ptB.y, target.x, target.y);

        if(testPtA < testPtB)
        {
            center += length;
            winPt = ptA;
        } else {
            center -= length;
            winPt = ptB;
        }
    }
    if(t)
        *t = center;

    return winPt;
}

//-------------------------------------------------------calc_intersection
bool calc_intersection(double ax, double ay, double bx, double by,
                                  double cx, double cy, double dx, double dy,
                                  double* x, double* y)
{
    double num = (ay-cy) * (dx-cx) - (ax-cx) * (dy-cy);
    double den = (bx-ax) * (dy-cy) - (by-ay) * (dx-cx);
    if(fabs(den) < intersection_epsilon) return false;
    double r = num / den;
    *x = ax + r * (bx-ax);
    *y = ay + r * (by-ay);
    return true;
}

//-------------------------------------------------------calc_intersection
bool calc_intersection(point_d a, point_d b,
                                  point_d c, point_d d,
                                  point_d *result)
{
    return calc_intersection(a.x,a.y,b.x,b.y,c.x,c.y,d.x,d.y,&result->x, &result->y);
}

//-------------------------------------------------------calc_intersection
bool calc_intersection(point_d a, double angleA,
                                  point_d b, double angleB,
                                  point_d *result)
{
    point_d _a{a.x + cos(qDegreesToRadians(angleA)), a.y + sin(qDegreesToRadians(angleA))};
    point_d _b{b.x + cos(qDegreesToRadians(angleB)), b.y + sin(qDegreesToRadians(angleB))};

    return calc_intersection(a,_a, b, _b, result);
}

}





#endif // MATHUTILS_H
