#include "bezier.h"
#include "beziermath.h"
#include "vektor/span.h"
#include "vektor/vektor.h"

namespace exo {

Bezier::Bezier(const PointVector &t_points):points(t_points)
{
    order = t_points.size() - 1;
}

Bezier::Bezier(const Span &t_span)
{
    m_isLinear = t_span.type() == Span::SpanTypeLine;
    if(m_isLinear)
    {
        points.push_back(t_span.pointA());
        points.push_back(t_span.pointB());
    }
    else
    {
        points.push_back(t_span.pointA());
        points.push_back(t_span.controlOut());
        points.push_back(t_span.controlIn());
        points.push_back(t_span.pointB());
    }
    order = points.size() - 1;

}

void Bezier::toSpan(Span &t_span, bool t_reverse) const
{
    t_span.setPointA(points[0]);
    t_span.setPointB(points[3]);
    if(t_reverse)
    {
        t_span.setControlOut(points[2]);
        t_span.setControlIn(points[1]);
    } else {
        t_span.setControlOut(points[1]);
        t_span.setControlIn(points[2]);
    }

    //t_span.recalculate();

    //t_graphic->points()[t_span.indexA()] = t_span.pointA();
    //t_graphic->points()[t_span.indexB()] = t_span.pointB();
}

void Bezier::calculate()
{
    if(m_isCalculated)
        return;

    auto aligned = BezierMath::aligned(points, line_d{points.front(), points.back()});
    m_isLinear = std::any_of(aligned.cbegin(), aligned.cend(),[](const point_d &pt){return abs(pt.y) > 0.0001;});

    m_lut.clear();
    m_dpoints = BezierMath::derive(points);
    computeDirection();
    m_isCalculated = true;
}

double Bezier::length() const
{
    if(m_length == 0.0)
    {
        m_length = BezierMath::length([this](double t){
            return BezierMath::compute(t, points);
        });
    }
    return m_length;
}

void Bezier::computeDirection()
{
    calculate();
    m_isClockwise = BezierMath::angle(points.front(), points.back(), points[1]);
}

point_d Bezier::compute(double t_t) const
{
    //TODO: Add ratio computing
    return BezierMath::compute(t_t, points);
}

std::vector<std::pair<double, double>> Bezier::intersects(const Bezier &t_curve) const
{
    if(t_curve.order > 1 && order > 1)
        return BezierMath::pairiteration(*this, t_curve);
    if(order > 1)
        return lineIntersects(line_d{t_curve.points[0], t_curve.points[1]});
    else
        return t_curve.lineIntersects(line_d{points[0], points[1]});
}

std::vector<std::pair<double, double>> Bezier::lineIntersects(const line_d &t_line) const
{
    std::vector<double> r;
    BezierMath::roots(points, r, t_line);

    bounds_d b = t_line.bounds();

    r.erase(std::remove_if(r.begin(), r.end(),[b, this](const double d){
        //return !overlaps(pair.first.bbox(),pair.second.bbox());
        return !b.contains(compute(d));
    }), r.end());

    std::sort(r.begin(), r.end());

    auto last = std::unique(r.begin(), r.end());
    r.erase(last, r.end());

    std::vector<std::pair<double, double>> results;
    for(double d : r)
    {
        results.push_back(std::pair<double,double>(d, line_d{compute(d),t_line.p1()}.length() / t_line.length()));
    }
    return results;
}

Bezier::ExtremaResults Bezier::extrema() const
{
    //calculate();

    ExtremaResults results;

    std::vector<double> roots;

    if(m_dpoints.empty())
    {
        return results;
    }

    BezierMath::drootsX(m_dpoints[0], results.xRoots);
    BezierMath::drootsY(m_dpoints[0], results.yRoots);

    if(order == 3)
    {
        BezierMath::drootsX(m_dpoints[1], results.xRoots);
        BezierMath::drootsY(m_dpoints[1], results.yRoots);
    }

    std::sort(results.xRoots.begin(), results.xRoots.end());
    std::sort(results.yRoots.begin(), results.yRoots.end());


    results.values = results.xRoots;
    results.values.insert(results.values.end(),results.yRoots.begin(), results.yRoots.end());
    std::sort(results.values.begin(), results.values.end());

    auto last = std::unique(results.values.begin(), results.values.end());
    results.values.erase(last, results.values.end());

    return results;
}

bounds_d Bezier::bounds() const
{
    if(!m_bounds.is_null())
        return m_bounds;
    m_bounds = bounds_d{};
    m_bounds.unite(points.front());
    m_bounds.unite(points.back());
    const auto extremaResults = extrema();
    for(double t : extremaResults.xRoots)
    {
        m_bounds.unite(compute(t));
    }
    for(double t : extremaResults.yRoots)
    {
        m_bounds.unite(compute(t));
    }
    return m_bounds;
}

bool Bezier::overlaps(const Bezier &t_other) const
{
    return bounds().intersects(t_other.bounds());
}

PointVector Bezier::hull(double t_t) const
{
    auto p = points;
    PointVector _p;
    PointVector q = p;

    // we lerp between all points at each iteration, until we have 1 point left.
    while (p.size() > 1) {
      _p.clear();
      point_d pt;
      for (int i = 0, l = p.size() - 1; i < l; i++) {
        pt = BezierMath::lerp(t_t, p[i], p[i + 1]);
        //q[idx++] = pt;
        q.push_back(pt);
        _p.push_back(pt);
      }
      p = _p;
    }
    return q;
}

SplitResult Bezier::split(double t_1, double t_2) const
{

    if(t_1 <= 0.0 && t_2 >= 0.0)
        return split(t_2).left;
    if(t_2 >= 1.0)
        return split(t_1).right;


    auto q = hull(t_1);
    SplitResult result;
    result.left = order == 2 ? Bezier({q[0], q[3], q[5]}) : Bezier({q[0], q[4], q[7], q[9]});
    result.right = order == 2 ? Bezier({q[5], q[4], q[2]}) : Bezier({q[9], q[8], q[6], q[3]});
    result.span = q;

    result.left.m_t1 = BezierMath::map(0, 0, 1, m_t1, m_t2);
    result.left.m_t2 = BezierMath::map(t_1, 0, 1, m_t1, m_t2);
    result.right.m_t1 = BezierMath::map(t_1, 0, 1, m_t1, m_t2);
    result.right.m_t2 = BezierMath::map(1, 0, 1, m_t1, m_t2);
    if(t_2 < 0.0)
        return result;

    t_2 = BezierMath::map(t_2, t_1, 1, 0, 1);
    return result.right.split(t_2).left;
}

point_d Bezier::derivative(double t_t) const
{
    return BezierMath::compute(t_t, m_dpoints[0]);
}

Bezier::OffsetResult Bezier::offset(double t_t, double t_d) const
{
    OffsetResult result;
    result.c = compute(t_t);
    result.n = normal(t_t);
    result.x = result.c.x + result.n.x * t_d;
    result.y = result.c.y + result.n.y * t_d;
    return result;
}

point_d Bezier::normal(double t_value) const
{
    point_d d = derivative(t_value);
    double q = sqrt(d.x * d.x + d.y * d.y);
    return point_d{-d.y / q, d.x / q };
}

void Bezier::scale(double t_value)
{
    std::vector<OffsetResult> v = {offset(0, 10), offset(1,10)};
    PointVector np;
    point_d o = BezierMath::lli4(v[0].toPoint(), v[0].c, v[1].toPoint(), v[1].c);

    points[0] += v[0].n * t_value;
    points[3] += v[1].n * t_value;

    point_d p = points[0];
    point_d d = derivative(0);
    point_d p2 =  point_d{ p.x + d.x, p.y + d.y };
    points[1] = BezierMath::lli4(p,p2,o, points[1]);


    p = points[3];
    d = derivative(1);
    p2 =  point_d{ p.x + d.x, p.y + d.y };
    points[2] = BezierMath::lli4(p,p2,o, points[2]);
}


} // deco
