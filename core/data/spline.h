#ifndef SPLINE_H
#define SPLINE_H
#include <QDataStream>
#include <QPainterPath>
#include "photon-global.h"
#include "types.h"

namespace photon {

class PHOTONCORE_EXPORT SplinePoint
{
public:
    enum SplineType
    {
        SplineCubic,
        SplineLinear,
        SplineStep
    };
    SplinePoint(SplineType type = SplineLinear):m_inType(type),m_outType(type){}
    SplinePoint(point_d position, SplineType type = SplineLinear):m_position(position),m_inType(type),m_outType(type){}
    SplinePoint(point_d position,point_d in,point_d out):m_position(position),m_in(in),m_out(out),m_inType(SplineCubic),m_outType(SplineCubic){}

    point_d position() const {return m_position;}
    point_d in() const {return m_in + m_position;}
    point_d out() const {return m_out + m_position;}
    point_d inRelative() const {return m_in;}
    point_d outRelative() const {return m_out;}
    SplineType outType() const{return m_outType;}
    void setOutType(const SplinePoint::SplineType type);

    void setPosition(const point_d &pt);
    void setIn(const point_d &pt, bool link = true);
    void setOut(const point_d &pt, bool link = true);
    void setInRelative(const point_d &pt);
    void setOutRelative(const point_d &pt);
    void setAutoTangents(bool value);

    bool autoTangents() const{return m_autoTangents;}
    bool xIsLocked() const{return m_lockX;}
    bool yIsLocked() const{return m_lockY;}
    bool tangentsAreLocked() const{return m_lockTangents;}
    void setXIsLocked(bool value){m_lockX = value;}
    void setYIsLocked(bool value){m_lockY = value;}
    void setTangentsAreLocked(bool value){m_lockTangents = value;}

    bool operator==(const SplinePoint &other) const;

    PHOTONCORE_EXPORT friend QDataStream & operator<< (QDataStream& stream, const SplinePoint &point)
    {
        unsigned short int version = 1;
        stream << version;
        stream << point.m_position;
        stream << point.m_in;
        stream << point.m_out;
        stream << point.m_lockX;
        stream << point.m_lockY;
        stream << point.m_lockTangents;
        stream << point.m_inType;
        stream << point.m_outType;
        stream << point.m_autoTangents;
        return stream;
    }
    PHOTONCORE_EXPORT friend QDataStream & operator>> (QDataStream& stream, SplinePoint &point)
    {
        unsigned short int version;
        int inType,outType;
        stream >> version;
        stream >> point.m_position;
        stream >> point.m_in;
        stream >> point.m_out;
        stream >> point.m_lockX;
        stream >> point.m_lockY;
        stream >> point.m_lockTangents;
        stream >> inType;
        stream >> outType;        
        stream >> point.m_autoTangents;
        point.m_inType = static_cast<SplineType>(inType);
        point.m_outType = static_cast<SplineType>(outType);
        return stream;
    }

private:

    friend class Spline;
    void setAutoIn(const point_d &pt);
    void setAutoOut(const point_d &pt);

    point_d m_position;
    point_d m_in;
    point_d m_out;
    SplineType m_inType;
    SplineType m_outType;
    bool m_lockX = false;
    bool m_lockY = false;
    bool m_lockTangents = true;
    bool m_autoTangents = true;
};

class PHOTONCORE_EXPORT Spline
{
public:
    Spline();
    Spline(const QVector<SplinePoint> &points);

    uint insert(const SplinePoint &pt);
    void remove(uint index);
    uint insertPointAtX(double x);

    bounds_d bounds() const;
    double value(double f, bool precise = false) const;
    void clear(){m_points.clear();}

    void removePointAtX(double position);
    bool hasPointAtX(double position) const;
    bool nextPoint(double position, uint *index);
    bool previousPoint(double position, uint *index);

    uint setPointPosition(uint index, point_d position);


    void sort();
    void recalculate();
    QPainterPath path() const;

    SplinePoint &first(){return m_points.front();}
    const SplinePoint &first() const{return m_points.front();}
    SplinePoint &last(){return m_points.back();}
    const SplinePoint &last() const{return m_points.back();}
    size_t pointCount() const {return m_points.size();}
    auto cbegin() const{return m_points.cbegin();}
    auto cend() const{return m_points.cend();}
    auto begin(){return m_points.begin();}
    auto end(){return m_points.end();}
    QVector<SplinePoint> &points(){return m_points;}

    bool operator==(const Spline &other) const;


    SplinePoint &at(size_t i){return m_points[i];}
    const SplinePoint &at(size_t i)const {return m_points[i];}
    SplinePoint &operator[](size_t i){return m_points[i];}
    const SplinePoint &operator[](size_t i) const{return m_points[i];}

    PHOTONCORE_EXPORT friend QDataStream & operator<< (QDataStream& stream, const Spline &curve);
    PHOTONCORE_EXPORT friend QDataStream & operator>> (QDataStream& stream, Spline &curve);

private:
    QVector<SplinePoint> m_points;
    unsigned m_outVertex = 0;
    unsigned m_subVertex = 0;
};

} // namespace deco

#endif // SPLINE_H
