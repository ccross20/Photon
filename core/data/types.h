#ifndef TYPES_H
#define TYPES_H

#include <math.h>
#include <string>
#include <QVariant>
#include <QDebug>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>

namespace photon {


//--------------------------------------------------------------point_base
template<class T> struct point_base
{
    typedef T value_type;
    T x = 0;
    T y = 0;
    point_base() {}
    point_base(T x_, T y_) : x(x_), y(y_) {}

    point_base(const point_base &other)
    {
        x = other.x;
        y = other.y;
    }

    point_base(const QPoint &other)
    {
        x = other.x();
        y = other.y();
    }

    point_base(const QPointF &other)
    {
        x = other.x();
        y = other.y();
    }

    template<typename FromT>
    point_base(const point_base<FromT> &other)
    {
        x = other.x;
        y = other.y;
    }

    std::string toString() const
    {
        return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    }

    bool isZero() const
    {
        return qFuzzyCompare(x, 0) && qFuzzyCompare(y, 0);
    }

    bool isValid() const
    {
        return !isNan();
    }

    bool isNan() const
    {
        return qIsNaN(x) || qIsNaN(y);
    }

    bool hitTest(const point_base<T> &p , double radius) const
    {
        return x - radius <= p.x && x + radius >= p.x &&
                y - radius <= p.y && y + radius >= p.y;
    }

    point_base<T> & operator=(const point_base<T> &other)
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    operator QPoint() const
    {
        return QPoint(x,y);
    }

    operator QPointF() const
    {
        return QPointF(x,y);
    }

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

    void round() {
        x = ::round(x);
        y = ::round(y);
    }

    point_base<T> rounded() const {
        return point_base<T>(::round(x), ::round(y));
    }


    point_base<T> &operator+=(const point_base<T> &p) {
        x += p.x;
        y += p.y;
        return *this;
    }

    point_base<T> operator+(const point_base<T> &v) const {
        return point_base<T>(x + v.x, y + v.y);
    }

    point_base<T> &operator-=(const point_base<T> &p) {
        x -= p.x;
        y -= p.y;
        return *this;
    }

    point_base<T> operator-(const point_base<T> &v) const {
        return point_base<T>(x - v.x, y - v.y);
    }

    point_base<T> &operator*=(const point_base<T> &p) {
        x *= p.x;
        y *= p.y;
        return *this;
    }

    point_base<T> &operator*=(const T &p) {
        x *= p;
        y *= p;
        return *this;
    }

    point_base<T> &operator/=(const T &p) {
        x /= p;
        y /= p;
        return *this;
    }

    point_base<T> operator*(const point_base<T> &v) const {
        return point_base<T>(x * v.x, y * v.y);
    }

    point_base<T> operator*(const T &v) const {
        return point_base<T>(x * v, y * v);
    }

    point_base<T> operator/(const T &v) const {
        return point_base<T>(x / v, y / v);
    }

    point_base<T> operator-() const {
        return point_base<T>(-x,-y);
    }

    bool appoximately(const point_base<T> &v, double margin) const
    {
        return abs(x - v.x) < margin && abs(y - v.y) < margin;
    }

    bool operator==(const point_base<T> &p) const { return x == p.x && y == p.y; }
    bool operator!=(const point_base<T> &p) const { return x != p.x || y != p.y; }

    friend QDebug operator<< (QDebug debug, const point_base<T> &pt)
    {
        QDebugStateSaver saver(debug);
        debug.nospace() << "Point (" << pt.x << "," << pt.y << ")";

        return debug;
    }

    friend QDataStream & operator<< (QDataStream& stream, const point_base<T> &pt)
    {
        stream << pt.x;
        stream << pt.y;
        return stream;
    }
    friend QDataStream & operator>> (QDataStream& stream, point_base<T> &pt)
    {
        stream >> pt.x;
        stream >> pt.y;
        return stream;
    }
};


typedef point_base<int>    point_i; //-----point_i
typedef point_base<float>  point_f; //-----point_f
typedef point_base<double> point_d; //-----point_d

template<>
inline bool point_i::isZero() const
{
    return x == 0 && y == 0;
}


//--------------------------------------------------------------size_base
template<class T> struct size_base
{
    typedef T value_type;
    T width = 0;
    T height = 0;
    size_base() {}
    size_base(T width_, T height_) : width(width_), height(height_) {}

    template<typename FromT>
    static size_base fromPoint(const point_base<FromT> &other)
    {
        return size_base(other.x,other.y);
    }

    size_base(const size_base &other)
    {
        width = other.width;
        height = other.height;
    }

    template<typename FromT>
    size_base(const size_base<FromT> &other)
    {
        width = other.width;
        height = other.height;
    }

    size_base<T> & operator=(const size_base<T> &other)
    {
        width = other.width;
        height = other.height;
        return *this;
    }


    std::string toString() const
    {
        return "(" + std::to_string(width) + "," + std::to_string(height) + ")";
    }

    void align()
    {
        width = ceil(width);
        height = ceil(height);
    }

    size_base aligned() const
    {
        return size_base(ceil(width), ceil(height));
    }

    QSize toQSize() const
    {
        return QSize(width, height);
    }

    QSizeF toQSizeF() const
    {
        return QSizeF(width, height);
    }

    size_base<T> &operator+=(const size_base<T> &p) {
        width += p.width;
        height += p.height;
        return *this;
    }

    size_base<T> operator+(const size_base<T> &v) const {
        return size_base<T>(width + v.width, height + v.height);
    }

    size_base<T> &operator-=(const size_base<T> &p) {
        width -= p.width;
        height -= p.height;
        return *this;
    }

    size_base<T> operator-(const size_base<T> &v) const {
        return size_base<T>(width - v.width, height - v.height);
    }



    size_base<T> &operator*=(const point_base<T> &p) {
        width *= p.x;
        height *= p.y;
        return *this;
    }

    size_base<T> &operator*=(const T &p) {
        width *= p;
        height *= p;
        return *this;
    }

    size_base<T> &operator/=(const T &p) {
        width /= p;
        height /= p;
        return *this;
    }

    size_base<T> operator*(const point_base<T> &v) const {
        return size_base<T>(width * v.x, height * v.y);
    }

    size_base<T> operator*(const T &v) const {
        return size_base<T>(width * v, height * v);
    }

    size_base<T> operator/(const T &v) const {
        return size_base<T>(width / v, height / v);
    }




    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }


    bool operator==(const size_base<T> &p) const { return width == p.width && height == p.height; }
    bool operator!=(const size_base<T> &p) const { return width != p.width || height != p.height; }

    friend QDebug operator<< (QDebug debug, const size_base<T> &pt)
    {
        QDebugStateSaver saver(debug);
        debug.nospace() << "Size (" << pt.width << "," << pt.height << ")";

        return debug;
    }

    friend QDataStream & operator<< (QDataStream& stream, const size_base<T> &pt)
    {
        stream << pt.width;
        stream << pt.height;
        return stream;
    }
    friend QDataStream & operator>> (QDataStream& stream, size_base<T> &pt)
    {
        stream >> pt.width;
        stream >> pt.height;
        return stream;
    }
};


typedef size_base<int>    size_i; //-----size_i
typedef size_base<float>  size_f; //-----size_f
typedef size_base<double> size_d; //-----size_d

//----------------------------------------------------------------bounds_base
template<class T> struct bounds_base
{
    typedef T            value_type;
    typedef bounds_base<T> self_type;
    T x1 = (std::numeric_limits<T>::max)();
    T y1 = (std::numeric_limits<T>::max)();
    T x2 = (std::numeric_limits<T>::lowest)();
    T y2 = (std::numeric_limits<T>::lowest)();

    bounds_base() {}
    bounds_base(T x1_, T y1_, T x2_, T y2_) :
        x1(x1_), y1(y1_), x2(x2_), y2(y2_) {normalize();}
    bounds_base(const QRect &other) :
        x1(other.x()), y1(other.y()), x2(other.x() + other.width()), y2(other.y() + other.height()) {normalize();}
    bounds_base(const QRectF &other) :
        x1(other.x()), y1(other.y()), x2(other.x() + other.width()), y2(other.y() + other.height()) {normalize();}
    bounds_base(point_base<T> pt, size_base<T> sz) : x1(pt.x),y1(pt.y),x2(pt.x + sz.width),y2(pt.y + sz.height){normalize();}
    bounds_base(point_base<T> pt, point_base<T> pt2) : x1(pt.x),y1(pt.y),x2(pt2.x),y2(pt2.y){normalize();}

    template<typename FromT>
    bounds_base(const bounds_base<FromT> &other)
    {
        if(other.is_valid())
        {
            x1 = other.x1;
            y1 = other.y1;
            x2 = other.x2;
            y2 = other.y2;
        }
    }


    bool operator==(const bounds_base<T> &b) const { return x1 == b.x1 && y1 == b.y1 && x2 == b.x2 && y2 == b.y2; }
    bool operator!=(const bounds_base<T> &b) const { return x1 != b.x1 || y1 != b.y1 || x2 != b.x2 || y2 != b.y2; }

    void init(T x1_, T y1_, T x2_, T y2_)
    {
        x1 = x1_; y1 = y1_; x2 = x2_; y2 = y2_;
    }

    self_type & operator=(const self_type &other)
    {
        x1 = other.x1;
        y1 = other.y1;
        x2 = other.x2;
        y2 = other.y2;
        return *this;
    }

    std::string toString() const
    {
        return "( Bounds x1: " + std::to_string(x1) + " y1: " + std::to_string(y1) + " x2: " + std::to_string(x2) + " y2: " + std::to_string(y2) + " )";
    }

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

    const self_type& normalize()
    {
        T t;
        if(x1 > x2) { t = x1; x1 = x2; x2 = t; }
        if(y1 > y2) { t = y1; y1 = y2; y2 = t; }
        return *this;
    }

    self_type normalized() const
    {
        T t_x1;
        T t_x2;
        T t_y1;
        T t_y2;
        if(x1 > x2) {
            t_x1 = x2;
            t_x2 = x1;
        }
        else {
            t_x1 = x1;
            t_x2 = x2;
        }

        if(y1 > y2) {
            t_y1 = y2;
            t_y2 = y1;
        }
        else {
            t_y1 = y1;
            t_y2 = y2;
        }
        return self_type(t_x1, t_y1, t_x2, t_y2);
    }

    bool clip(const self_type& r)
    {
        if(x2 > r.x2) x2 = r.x2;
        if(y2 > r.y2) y2 = r.y2;
        if(x1 < r.x1) x1 = r.x1;
        if(y1 < r.y1) y1 = r.y1;
        return x1 <= x2 && y1 <= y2;
    }

    bool is_valid() const
    {
        return x1 <= x2 && y1 <= y2;
    }

    bool is_empty() const
    {
        return x1 >= x2 && y1 >= y2;
    }

    bool is_null() const
    {
        return x1 <= (std::numeric_limits<T>::max()) || y1 <= (std::numeric_limits<T>::max()) || x2 >= (std::numeric_limits<T>::lowest()) || y2 >= (std::numeric_limits<T>::lowest());
    }

    bool hit_test(T x, T y) const
    {
        if(!is_valid())
            return false;
        return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
    }

    bool hit_test_inside(T x, T y) const
    {
        if(!is_valid())
            return false;
        return (x > x1 && x < x2 && y > y1 && y < y2);
    }

    bool contains(const point_base<T> &pt) const
    {
        if(!is_valid())
            return false;
        return (pt.x >= x1 && pt.x <= x2 && pt.y >= y1 && pt.y <= y2);
    }

    /*
    bool contains(const rect_base<T> &rect) const
    {
        if(!is_valid() || !rect.is_valid())
            return false;
        return hit_test(rect.x1, rect.y1) && hit_test(rect.x2, rect.y2);
    }
    */

    bool contains(const bounds_base<T> &bnds) const
    {
        if(!is_valid() || !bnds.is_valid())
            return false;


        return hit_test(bnds.x1, bnds.y1) && hit_test(bnds.x2, bnds.y2);
    }

    /*
    bool intersects(const rect_base<T> &rect) const
    {
        if(!is_valid() || !rect.is_valid())
            return false;
        return hit_test(rect.x1, rect.y1) || hit_test(rect.x2, rect.y2);
    }
    */

    bool intersects(const bounds_base<T> &bnds) const
    {
        if(!is_valid() || !bnds.is_valid())
            return false;
        return x2 >= bnds.x1 && x1 <= bnds.x2 && y2 >= bnds.y1 && y1 <= bnds.y2;
        //return hit_test(bnds.x1, bnds.y1) || hit_test(bnds.x2, bnds.y2);
    }

    bool overlaps(const bounds_base<T> &bnds) const
    {
        if(!is_valid() || !bnds.is_valid())
            return false;
        return x2 > bnds.x1 && x1 < bnds.x2 && y2 > bnds.y1 && y1 < bnds.y2;
        //return hit_test(bnds.x1, bnds.y1) || hit_test(bnds.x2, bnds.y2);
    }

    T width() const
    {
        return (x2 - x1);
    }

    T height() const
    {
        return (y2 - y1);
    }

    size_base<T> size() const
    {
        return size_base<T>(width(), height());
    }

    T top() const
    {
        return y1;
    }

    T bottom() const
    {
        return y2;
    }

    T left() const
    {
        return x1;
    }

    T x() const
    {
        return x1;
    }

    T y() const
    {
        return y1;
    }

    T right() const
    {
        return x2;
    }

    point_base<T> center() const
    {
        return point_base<T>((width()/2.0)+x1,(height()/2.0)+y1);
    }

    point_base<T> topLeft() const
    {
        return point_base<T>(x1,y1);
    }

    point_base<T> topRight() const
    {
        return point_base<T>(x2,y1);
    }

    point_base<T> bottomLeft() const
    {
        return point_base<T>(x1,y2);
    }

    point_base<T> bottomRight() const
    {
        return point_base<T>(x2,y2);
    }

    void alignWithin()
    {
        if(!is_valid())
            return;

            x1 = ceil(x1);

            y1 = ceil(y1);

            x2 = floor(x2);
            y2 = floor(y2);
    }

    self_type alignedWithin() const
    {
        self_type r = self_type(x1, y1, x2, y2);
        r.alignWithin();
        return r;
    }

    void translate(const point_base<T> &pt)
    {
        if(!is_valid())
            return;

        x1 += pt.x;
        x2 += pt.x;
        y1 += pt.y;
        y2 += pt.y;
    }

    self_type translated(const point_base<T> &pt) const
    {
        self_type r = self_type(x1, y1, x2, y2);
        r.translate(pt);
        return r;
    }

    void translate(T x, T y)
    {
        if(!is_valid())
            return;
        x1 += x;
        x2 += x;
        y1 += y;
        y2 += y;
    }

    self_type translated(T x, T y) const
    {
        self_type r = self_type(x1, y1, x2, y2);
        r.translate(x,y);
        return r;
    }

    void align()
    {
        if(!is_valid())
            return;

        x1 = floor(x1);

        y1 = floor(y1);

        x2 = ceil(x2);
        y2 = ceil(y2);
        /*
        if(x1 > 0)
            x1 = floor(x1);
        else
            x1 = ceil(x1);

        if(y1 > 0)
            y1 = floor(y1);
        else
            y1 = ceil(y1);

        if(x2 < 0)
            x2 = floor(x2);
        else
            x2 = ceil(x2);

        if(y2 < 0)
            y2 = floor(y2);
        else
            y2 = ceil(y2);
            */
    }

    self_type aligned() const
    {
        self_type r = self_type(x1, y1, x2, y2);
        r.align();
        return r;
    }

    const self_type& unite(const self_type &other){
        if(!other.is_valid())
            return *this;

        if(x1 > other.x1) x1 = other.x1;
        if(y1 > other.y1) y1 = other.y1;
        if(x2 < other.x1) x2 = other.x1;
        if(y2 < other.y1) y2 = other.y1;

        if(x1 > other.x2) x1 = other.x2;
        if(y1 > other.y2) y1 = other.y2;
        if(x2 < other.x2) x2 = other.x2;
        if(y2 < other.y2) y2 = other.y2;
/*
*
* if(x2 > other.x2) x2 = other.x2;
        if(y2 > other.y2) y2 = other.y2;
        if(x1 < other.x1) x1 = other.x1;
        if(y1 < other.y1) y1 = other.y1;
*
        if(x2 > other.x1) x2 = other.x1;
        if(y2 > other.y1) y2 = other.y1;
        if(x1 < other.x2) x1 = other.x2;
        if(y1 < other.y2) y1 = other.y2;
        */
        return *this;
    }

    const self_type& unite(const point_base<T> &pt){
        if(x1 > pt.x) x1 = pt.x;
        if(y1 > pt.y) y1 = pt.y;
        if(x2 < pt.x) x2 = pt.x;
        if(y2 < pt.y) y2 = pt.y;
        return *this;
    }

    const self_type& adjust(T x1_, T y1_, T x2_, T y2_)
    {
        if(!is_valid())
            return *this;
        x1 += x1_;
        y1 += y1_;
        x2 += x2_;
        y2 += y2_;
        return *this;
    }

    const self_type& offset(T x1_)
    {
        if(!is_valid())
            return *this;
        x1 -= x1_;
        y1 -= x1_;
        x2 += x1_;
        y2 += x1_;
        return *this;
    }

    operator QRect() const
    {
        return QRect(x1,y1,width(),height());
    }

    operator QRectF() const
    {
        return QRectF(x1,y1,width(),height());
    }

    QRect toQRect() const
    {
        return QRect(x1,y1,width(),height());
    }

    QRectF toQRectF() const
    {
        return QRectF(x1,y1,width(),height());
    }

    self_type adjusted(T x1_, T y1_, T x2_, T y2_) const
    {
        if(!is_valid())
            return self_type{};
        return self_type(x1 + x1_, y1 + y1_, x2 + x2_, y2 + y2_);
    }

    self_type offsetted(T x1_) const
    {
        if(!is_valid())
            return self_type{};
        return self_type(x1 - x1_, y1 - x1_, x2 + x1_, y2 + x1_);
    }

    self_type united(const self_type &other) const
    {
        self_type r(*this);
        r.unite(other);
        return r;
    }

    self_type united(const point_base<T> &pt) const
    {
        self_type r(*this);
        r.unite(pt);
        return r;
    }

    friend QDebug operator<< (QDebug debug, const self_type &rect)
    {
        QDebugStateSaver saver(debug);
        debug.nospace() << QString::fromStdString(rect.toString());

        return debug;
    }

    friend QDataStream & operator<< (QDataStream& stream, const self_type &rect)
    {
        stream << rect.x1;
        stream << rect.y1;
        stream << rect.x2;
        stream << rect.y2;
        return stream;
    }
    friend QDataStream & operator>> (QDataStream& stream, self_type &rect)
    {
        stream >> rect.x1;
        stream >> rect.y1;
        stream >> rect.x2;
        stream >> rect.y2;
        return stream;
    }
};

//-----------------------------------------------------intersect_rectangles
template<class Bounds>
inline Bounds intersect_bounds(const Bounds& r1, const Bounds& r2)
{
    Bounds r = r1;

    // First process x2,y2 because the other order
    // results in Internal Compiler Error under
    // Microsoft Visual C++ .NET 2003 69462-335-0000007-18038 in
    // case of "Maximize Speed" optimization option.
    //-----------------

    if(r.x2 > r2.x2) r.x2 = r2.x2;
    if(r.y2 > r2.y2) r.y2 = r2.y2;
    if(r.x1 < r2.x1) r.x1 = r2.x1;
    if(r.y1 < r2.y1) r.y1 = r2.y1;

    return r;
}


//---------------------------------------------------------unite_rectangles
template<class Bounds>
inline Bounds unite_bounds(const Bounds& r1, const Bounds& r2)
{
    Bounds r = r1;
    if(r.x2 < r2.x2) r.x2 = r2.x2;
    if(r.y2 < r2.y2) r.y2 = r2.y2;
    if(r.x1 > r2.x1) r.x1 = r2.x1;
    if(r.y1 > r2.y1) r.y1 = r2.y1;
    return r;
}

typedef bounds_base<int>    bounds_i; //----rect_i
typedef bounds_base<float>  bounds_f; //----rect_f
typedef bounds_base<double> bounds_d; //----rect_d


}

#endif // TYPES_H
