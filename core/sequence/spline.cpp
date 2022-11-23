#include <algorithm>
#include "spline.h"

namespace photon {


bool SplinePoint::operator==(const SplinePoint &other) const
{
    return m_position == other.m_position &&
            m_in == other.m_in &&
            m_out == other.m_out &&
            m_lockX == other.m_lockX &&
            m_lockY == other.m_lockY &&
            m_lockTangents == other.m_lockTangents &&
            m_inType == other.m_inType &&
            m_outType == other.m_outType &&
            m_autoTangents == other.m_autoTangents;
}

void SplinePoint::setPosition(const QPointF &pt)
{
    if(!m_lockX)
        m_position.setX(pt.x());
    if(!m_lockY)
        m_position.setY(pt.y());
}

void SplinePoint::setIn(const QPointF &pt, bool link)
{
    double inAngleBefore = atan2(m_in.y(), m_in.x());
    setInRelative(pt - m_position);
    if(m_lockTangents && link)
    {
        double inAngleAfter = atan2(m_in.y(), m_in.x());
        double outAngle = atan2(m_out.y(), m_out.x()) + (inAngleAfter - inAngleBefore);
        double length = QLineF(QPointF(), m_out).length();
        m_out = (QPointF(cos(outAngle) * length,sin(outAngle) * length));
    }
    m_autoTangents = false;
}

void SplinePoint::setOut(const QPointF &pt, bool link)
{
    double outAngleBefore = atan2(m_out.y(), m_out.x());
    setOutRelative(pt - m_position);
    if(m_lockTangents && link)
    {
        double outAngleAfter = atan2(m_out.y(), m_out.x());
        double inAngle = atan2(m_in.y(), m_in.x()) + (outAngleAfter - outAngleBefore);
        double length = QLineF(0,0,m_in.x(), m_in.y()).length();
        setInRelative(QPointF(cos(inAngle) * length,sin(inAngle) * length));
    }
    m_autoTangents = false;
}

void SplinePoint::setAutoIn(const QPointF &pt)
{
    setInRelative(pt);
}

void SplinePoint::setAutoOut(const QPointF &pt)
{
    setOutRelative(pt);
}

void SplinePoint::setAutoTangents(bool value)
{
    m_autoTangents = value;
}

void SplinePoint::setOutType(const SplinePoint::SplineType type)
{
    m_outType = type;
}

void SplinePoint::setInRelative(const QPointF &pt)
{
    m_in = pt;
    if(m_in.x() > 0)
        m_in.setX(0);
}

void SplinePoint::setOutRelative(const QPointF &pt)
{
    m_out = pt;
    if(m_out.x() < 0)
        m_out.setX(0);
}

Spline::Spline()
{

}

Spline::Spline(const std::vector<SplinePoint> &points) : m_points(points)
{

}

QPainterPath Spline::path() const
{
    QPainterPath resultPath;

    if(m_points.size() <= 0)
        return resultPath;

    auto it = m_points.cbegin();
    resultPath.moveTo(QPointF((*it).position().x(),(*it).position().y()));
    SplinePoint lastPt = (*it);
    uint index = 1;
    for(++it; it != m_points.cend(); ++it)
    {
        switch (lastPt.outType()) {
            case SplinePoint::SplineLinear:
            {
                resultPath.lineTo(QPointF((*it).position().x(),(*it).position().y()));
            }
            break;
            case SplinePoint::SplineCubic:
            {
                QPointF outAbs = lastPt.out();
                QPointF inAbs = (*it).in();

                outAbs.setX(qMin(outAbs.x(), (*it).position().x()));
                inAbs.setX(qMax(inAbs.x(), lastPt.position().x()));
                resultPath.cubicTo(QPointF(outAbs.x(),outAbs.y()),QPointF(inAbs.x(),inAbs.y()),QPointF((*it).position().x(),(*it).position().y()));
            }
            break;
            case SplinePoint::SplineStep:
            {
                resultPath.lineTo(QPointF((*it).position().x(),lastPt.position().y()));
                resultPath.lineTo(QPointF((*it).position().x(),(*it).position().y()));
            }
            break;
        }
        lastPt = *it;
        ++index;
    }

    /*
    int lastX = m_points.back().position().x;

    resultPath.moveTo(m_points.front().position());
    for(int i = m_points.cbegin()->position().x+1; i < lastX; ++i)
    {
        resultPath.lineTo(QPointF(i, value(i)));
    }
    */




    return resultPath;
}

bool Spline::nextPoint(double position, uint *index)
{
    if(m_points.size() <= 0)
        return false;
    if(m_points.back().position().x() <= position)
        return false;
    uint counter = 0;
    for(auto &pt : m_points)
    {
        if(pt.position().x() > position)
        {
            *index = counter;
            return true;
        }
        ++counter;
    }
    return false;
}

bool Spline::previousPoint(double position, uint *index)
{
    if(m_points.size() <= 0)
        return false;
    if(m_points.front().position().x() >= position)
        return false;

    uint counter = m_points.size() - 1;
    for(auto it = m_points.crbegin(); it != m_points.crend(); ++it)
    {
        const SplinePoint &pt = *it;
        if(pt.position().x() < position)
        {
            *index = counter;
            return true;
        }
        --counter;
    }
    return false;
}

void Spline::sort()
{
    std::sort(m_points.begin(), m_points.end(),[](const SplinePoint &a, const SplinePoint &b){return a.position().x() < b.position().x();});
}

void Spline::recalculate()
{
    if(m_points.size() <= 1)
        return;

    auto it = m_points.begin();
    SplinePoint *lastPoint = &*it;
    ++it;

    for(; it != m_points.end(); ++it)
    {
        SplinePoint &thisPoint = *it;

        double delta = thisPoint.position().x() - lastPoint->position().x();
        if(lastPoint->autoTangents())
        {
            lastPoint->setAutoOut(QPointF{delta * .33, 0});
        }

        if(thisPoint.autoTangents())
        {
            thisPoint.setAutoIn(QPointF{-delta * .33, 0});
        }

        lastPoint = &thisPoint;
    }
}


uint Spline::insert(const SplinePoint &pt)
{
    uint index = 0;
    for(auto it = m_points.begin(); it != m_points.end(); ++it, ++index)
    {
        if((*it).position().x() >= pt.position().x())
        {
            if(qFuzzyCompare((*it).position().x(), pt.position().x()))
                (*it).setPosition(pt.position());
            else
                m_points.insert(it, pt);
            return index;
        }
    }
    m_points.push_back(pt);

    return static_cast<uint>(m_points.size()) - 1;
}

uint Spline::insertPointAtX(double x)
{
    if(m_points.empty())
    {
        m_points.push_back(SplinePoint(QPointF(x,0)));
        return 0;
    }


    if(x < m_points.front().position().x())
    {
        m_points.insert(m_points.begin(), SplinePoint(QPointF(x,m_points.front().position().y()), m_points.front().outType()));
        return 0;
    }

    if(x > m_points.back().position().x())
    {
        m_points.push_back(SplinePoint(QPointF(x,m_points.back().position().y()), m_points.back().outType()));
        return m_points.size() - 1;
    }


    uint index = 0;
    uint before = 0;
    uint after = 0;
    for(auto it = m_points.cbegin(); it != m_points.cend(); ++it, ++index)
    {
        if((*it).position().x() >= x)
        {
            before = index-1;
            after = index;
            break;
        }
    }

    SplinePoint &beforePt = m_points[before];
    SplinePoint &afterPt = m_points[after];

    if(beforePt.position().x() == x || afterPt.position().x() == x)
    {
        return 0;
    }

    if(beforePt.outType() == SplinePoint::SplineLinear)
    {
        double y = value(x);
        SplinePoint midSplinePt(QPointF(x,y), SplinePoint::SplineLinear);



        m_points.insert(m_points.begin() + after,midSplinePt);
        //midSplinePt.setInType(beforePt.outType());
    }
    else if(beforePt.outType() == SplinePoint::SplineStep)
    {
        double y = value(x);
        SplinePoint midSplinePt(QPointF(x,y), SplinePoint::SplineStep);

        m_points.insert(m_points.begin() + after,midSplinePt);
    }
    else if(beforePt.outType() == SplinePoint::SplineCubic)
    {
        /*
        QPointF out = beforePt.out();
        QPointF in = afterPt.in();

        double y = value(x);
        double t;
        calc_bezier_closest_point(beforePt.position(), out, in, afterPt.position(),QPointF(x,y), &t);


        double x12 = (out.x - beforePt.position().x)*t + beforePt.position().x;
        double y12 = (out.y - beforePt.position().y)*t + beforePt.position().y;

        double x23 = (in.x - out.x) * t + out.x;
        double y23 = (in.y - out.y) * t + out.y;

        double x34 = (afterPt.position().x - in.x) * t + in.x;
        double y34 = (afterPt.position().y - in.y) * t + in.y;

        double x123 = (x23-x12)*t+x12;
        double y123 = (y23-y12)*t+y12;

        double x234 = (x34-x23)*t+x23;
        double y234 = (y34-y23)*t+y23;

        double y1234 = (y234-y123)*t+y123;

        QPointF midPt = QPointF(x,y1234);


        beforePt.setOutRelative(QPointF(x12,y12) - beforePt.position());

        SplinePoint midSplinePt(midPt,QPointF(x123,y123) - midPt,QPointF(x234,y234) - midPt);
        midSplinePt.setAutoTangents(false);

        afterPt.setInRelative(QPointF(x34,y34) - afterPt.position());




        m_points.insert(m_points.begin() + after,midSplinePt);
        //midSplinePt.setInType(beforePt.outType());
        midSplinePt.setOutType(beforePt.outType());
        */
    }



    return after;
}

void Spline::remove(uint index)
{
    if(index > 0 && index < m_points.size())
        m_points.erase(m_points.cbegin() + index);
}

static double root_find_y(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, const double x)
{
    //return BezierMath::yForX(a,b,c,d,x);
    return 0.0;
}

double test_calc_bezier_y(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d, const double x)
{
    /*
    uint iterations = 6;

    double ptA;
    double ptB;

    double center = .5;
    double length = .5;
    for(uint i = 0; i < iterations; ++i)
    {
        length *= .5;

        ptA = calc_bezier(a.x, b.x, c.x, d.x, center + length);
        ptB = calc_bezier(a.x, b.x, c.x, d.x, center - length);

        ptA = (ptA - x) * (ptA - x);
        ptB = (ptB - x) * (ptB - x);

        if(ptA < ptB)
        {
            center += length;
        } else if (ptA > ptB ){
            center -= length;
        }
        else
            break;
    }

    double leftX = center - length;
    double rightX = center + length;
    double deltaX = rightX - leftX;

    double multiple = (x - leftX) / deltaX;

    double leftY = calc_bezier(a.y, b.y, c.y, d.y, center - length);
    double rightY = calc_bezier(a.y, b.y, c.y, d.y, center + length);
    double deltaY = rightY - leftY;
    return (deltaY * multiple) + leftY;

    //return  - calc_bezier(a.y, b.y, c.y, d.y, center - length);
    */
    return 0.0;

}

double Spline::value(double f, bool precise) const
{
    if(m_points.empty())
        return 0.0;
    if(f <= m_points.front().position().x())
        return m_points.front().position().y();
    else if(f >= m_points.back().position().x())
        return m_points.back().position().y();


    for(auto it = m_points.cbegin()+1; it != m_points.cend(); ++it)
    {
        if((*it).position().x() >= f)
        {

            const SplinePoint &pt = (*(it-1));
            if(qFuzzyCompare((*it).position().x(), f))
            {
                return (*it).position().y();
            }

            const SplinePoint &nextPt = *it;

            //double localF = (f - pt.position().x)/(nextPt.position().x - pt.position().x);

            if(pt.outType() == SplinePoint::SplineCubic)
            {

                QPointF a = pt.position();
                QPointF b = QPointF(qMin(pt.out().x(), nextPt.position().x()),pt.out().y());
                QPointF c = QPointF(qMax(nextPt.in().x(), pt.position().x()),nextPt.in().y());
                QPointF d = nextPt.position();

                double x = f;

                return root_find_y(a,b,c,d,x);

                //return (calc_bezier_y(a,b,c,d,x-.001, 24) + calc_bezier_y(a,b,c,d,x, 24) + calc_bezier_y(a,b,c,d,x, 24+.001))/3.0;

                //qDebug() << x << "   " << intersect.y;

            }
            else if(pt.outType() == SplinePoint::SplineLinear)
            {

                QPointF a = pt.position();
                QPointF b = nextPt.position();
                QPointF intersectionPoint;
                QLineF line(a, b);
                line.intersects(QLineF(f,5000000,f,-5000000), &intersectionPoint);
                return intersectionPoint.y();
            }
            else {
                return pt.position().y();
            }
        }
    }
    return m_points.back().position().y();
}

uint Spline::setPointPosition(uint index, QPointF position)
{
    /*
    if(position.x > 1.0)
        position.x = 1.0;
    else if(position.x < 0.0)
        position.x = 0.0;

    if(position.y > 1.0)
        position.y = 1.0;
    else if(position.y < 0.0)
        position.y = 0.0;
        */

    SplinePoint &pt = m_points[index];
    pt.setPosition(position);



    if(index < m_points.size()-2 && pt.position().x() > m_points[index+1].position().x())
    {
        std::swap(pt, m_points[index+1]);
        return index + 1;
    }
    else if(index > 0 && pt.position().x() < m_points[index-1].position().x())
    {
        std::swap(pt, m_points[index-1]);
        return index-1;
    }
    return index;
}

void Spline::removePointAtX(double position)
{
    if(m_points.size() == 0)
        return;
    auto it = std::lower_bound(m_points.begin(),m_points.end(),position,  [](const SplinePoint &a, const double &b){return a.position().x() < b;});
    if(it == m_points.end())
        return;
    if(qFuzzyCompare((*it).position().x(), position))
    {
        m_points.erase(it);
    }
}

bool Spline::hasPointAtX(double position) const
{
    if(m_points.size() == 0)
        return false;
    auto it = std::lower_bound(m_points.cbegin(),m_points.cend(),position,  [](const SplinePoint &a, const double &b){return a.position().x() < b;});
    if(it == m_points.end())
        return false;
    return qFuzzyCompare((*it).position().x(), position);
}

bool Spline::operator==(const Spline &other) const
{
    return m_points == other.m_points;
}

QRectF Spline::bounds() const
{
    double x1,y1,x2,y2;


    for(const SplinePoint &splinePt : m_points)
    {
        const QPointF pt = splinePt.position();
        if(x1 > pt.x()) x1 = pt.x();
        if(y1 > pt.y()) y1 = pt.y();
        if(x2 < pt.x()) x2 = pt.x();
        if(y2 < pt.y()) y2 = pt.y();

    }

    return QRectF(QPointF(x1,y1), QPointF(x2,y2));
}

QDataStream & operator<< (QDataStream& stream, const Spline &curve)
{
    unsigned short int version = 1;
    stream << version;
    stream << QVector<SplinePoint>{curve.m_points.cbegin(),curve.m_points.cend()};
    return stream;
}

QDataStream & operator>> (QDataStream& stream, Spline &curve)
{
    unsigned short int version;
    QVector<SplinePoint> vect;
    stream >> version;
    stream >> vect;
    curve.m_points = std::vector<SplinePoint>{vect.cbegin(), vect.cend()};
    return stream;
}

} // namespace exo
