#ifndef POMAX_BEZIER_H
#define POMAX_BEZIER_H
#include "agg/agg_basics.h"
#include "exo-carbon_global.h"

namespace exo {

using PointVector = std::vector<point_d>;
using DoubleMatrix = std::vector<std::vector<double>>;

struct SplitResult;

class EXOCARBON_EXPORT Bezier
{
public:


    struct ExtremaResults
    {
        std::vector<double> xRoots;
        std::vector<double> yRoots;
        std::vector<double> values;
    };

    struct OffsetResult
    {
        point_d n,c;
        double x,y;

        point_d toPoint() const{return point_d{x,y};};
    };

    Bezier(const PointVector &t_points);
    Bezier(const Span &t_span);
    Bezier():order(0){}

    void computeDirection();
    point_d compute(double t_t) const;
    bool overlaps(const Bezier &t_other) const;
    bool isLinear() const{return m_isLinear;}
    SplitResult split(double t_1, double t_2 = -1.0) const;
    bool isNull() const{return points.empty();}
    bool isClockwise() const{return m_isClockwise;}
    double length() const;
    PointVector hull(double t_t) const;
    void toSpan(Span &t_span, bool t_reverse = false) const;

    void scale(double t_value);
    point_d normal(double t_value) const;
    OffsetResult offset(double t_t, double t_d) const;

    std::vector<std::pair<double, double>> intersects(const Bezier &t_curve) const;
    std::vector<std::pair<double, double>> lineIntersects(const line_d &t_line) const;

    bounds_d bounds() const;
    PointVector points;
    uint order;

    double t1() const {return m_t1;}
    double t2() const {return m_t2;}



private:
    void calculate();
    ExtremaResults extrema() const;
    point_d derivative(double t_t) const;

    mutable bounds_d m_bounds;
    std::vector<double> m_lut;
    std::vector<PointVector> m_dpoints;
    double m_t1 = 0.0;
    double m_t2 = 1.0;
    mutable double m_length = 0.0;
    bool m_isLinear;
    bool m_isClockwise;
    bool m_isCalculated = false;

};

struct SplitResult
{
    SplitResult(){}
    SplitResult(const Bezier &t_bezier):left(t_bezier){}
    Bezier left;
    Bezier right;
    PointVector span;
};

} // deco

#endif // POMAX_BEZIER_H
