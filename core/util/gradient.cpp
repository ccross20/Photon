#include "gradient.h"
#include "util/utils.h"

#include <QImage>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <cmath>

namespace photon {

Gradient::Gradient()
{
    m_stops = { {0.0, QColor(Qt::black)}, {1.0, QColor(Qt::white)} };
}

Gradient::Gradient(const QVector<GradientStop> &stops) : m_stops(stops)
{
    sort();
}

void Gradient::setStops(const QVector<GradientStop> &stops)
{
    m_stops = stops;
    sort();
}

void Gradient::sort()
{
    std::stable_sort(m_stops.begin(), m_stops.end(),
                     [](const GradientStop &a, const GradientStop &b){ return a.position < b.position; });
}

int Gradient::addStop(double position, const QColor &color)
{
    position = qBound(0.0, position, 1.0);
    m_stops.append({position, color});
    sort();

    for(int i = 0; i < m_stops.size(); ++i)
        if(m_stops[i].color == color && qFuzzyCompare(m_stops[i].position + 1.0, position + 1.0))
            return i;
    return m_stops.size() - 1;
}

void Gradient::removeStop(int index)
{
    if(index >= 0 && index < m_stops.size())
        m_stops.removeAt(index);
}

void Gradient::setStopColor(int index, const QColor &color)
{
    if(index >= 0 && index < m_stops.size())
        m_stops[index].color = color;
}

int Gradient::setStopPosition(int index, double position)
{
    if(index < 0 || index >= m_stops.size())
        return index;

    position = qBound(0.0, position, 1.0);
    GradientStop moved = m_stops.at(index);
    moved.position = position;
    m_stops[index].position = position;
    sort();

    // The moved stop is uniquely identifiable by matching both fields; find it.
    for(int i = 0; i < m_stops.size(); ++i)
        if(m_stops[i].color == moved.color && qFuzzyCompare(m_stops[i].position + 1.0, moved.position + 1.0))
            return i;
    return index;
}

QColor Gradient::colorAt(double t) const
{
    if(m_stops.isEmpty())
        return QColor(Qt::black);

    if(t <= m_stops.first().position)
        return m_stops.first().color;
    if(t >= m_stops.last().position)
        return m_stops.last().color;

    for(int i = 1; i < m_stops.size(); ++i)
    {
        const GradientStop &b = m_stops[i];
        if(t <= b.position)
        {
            const GradientStop &a = m_stops[i - 1];
            const double span = b.position - a.position;
            const double f = span > 1e-9 ? (t - a.position) / span : 0.0;

            const auto lerp = [f](int lo, int hi){ return qRound(lo + (hi - lo) * f); };
            return QColor(lerp(a.color.red(),   b.color.red()),
                          lerp(a.color.green(), b.color.green()),
                          lerp(a.color.blue(),  b.color.blue()),
                          lerp(a.color.alpha(), b.color.alpha()));
        }
    }
    return m_stops.last().color;
}

Gradient Gradient::reversed() const
{
    QVector<GradientStop> out;
    out.reserve(m_stops.size());
    for(const GradientStop &s : m_stops)
        out.append({1.0 - s.position, s.color});
    return Gradient(out);
}

Gradient Gradient::remapped(double offset, double scale, bool repeat) const
{
    if(m_stops.isEmpty())
        return *this;

    if(std::abs(scale) < 1e-4)
        scale = scale < 0.0 ? -1e-4 : 1e-4;

    const auto wrap01 = [](double x){ x = std::fmod(x, 1.0); if(x < 0.0) x += 1.0; return x; };

    QVector<GradientStop> out;
    const int guard = 256;   // cap emitted stops so a tiny scale can't blow up

    if(repeat)
    {
        // Sample new(t) = colorAt(frac((t - offset)/scale)). Emit a stop at every t
        // in [0,1] that lands on an original stop, preserving hard edges, plus
        // anchored endpoints so the visible window is fully covered.
        for(int i = 0; i < m_stops.size() && out.size() < guard; ++i)
        {
            const double p = m_stops[i].position;
            const QColor c = m_stops[i].color;
            const double a = ((0.0 - offset) / scale) - p;
            const double b = ((1.0 - offset) / scale) - p;
            const double kmin = std::ceil(std::min(a, b));
            const double kmax = std::floor(std::max(a, b));
            for(double k = kmin; k <= kmax && out.size() < guard; k += 1.0)
            {
                const double t = offset + scale * (k + p);
                out.append({qBound(0.0, t, 1.0), c});
            }
        }
        out.append({0.0, colorAt(wrap01((0.0 - offset) / scale))});
        out.append({1.0, colorAt(wrap01((1.0 - offset) / scale))});
    }
    else
    {
        for(int i = 0; i < m_stops.size(); ++i)
        {
            const double np = m_stops[i].position * scale + offset;
            out.append({qBound(0.0, np, 1.0), m_stops[i].color});
        }
        out.append({0.0, colorAt(qBound(0.0, (0.0 - offset) / scale, 1.0))});
        out.append({1.0, colorAt(qBound(0.0, (1.0 - offset) / scale, 1.0))});
    }

    return Gradient(out);
}

Gradient Gradient::mixed(const Gradient &a, const Gradient &b, double factor)
{
    factor = qBound(0.0, factor, 1.0);

    // Union of both gradients' stop positions gives the breakpoints where either
    // input changes slope; blend the sampled colours at each.
    QVector<double> positions;
    positions.reserve(a.count() + b.count());
    for(const GradientStop &s : a.stops()) positions.append(s.position);
    for(const GradientStop &s : b.stops()) positions.append(s.position);
    if(positions.isEmpty())
        return Gradient(QVector<GradientStop>());
    std::sort(positions.begin(), positions.end());

    QVector<GradientStop> out;
    double last = -1.0;
    for(double p : positions)
    {
        if(!out.isEmpty() && qAbs(p - last) < 1e-6)
            continue;   // dedupe coincident positions
        last = p;

        const QColor ca = a.colorAt(p);
        const QColor cb = b.colorAt(p);
        const auto lerp = [factor](int lo, int hi){ return qRound(lo + (hi - lo) * factor); };
        out.append({p, QColor(lerp(ca.red(),   cb.red()),
                              lerp(ca.green(), cb.green()),
                              lerp(ca.blue(),  cb.blue()),
                              lerp(ca.alpha(), cb.alpha()))});
    }
    return Gradient(out);
}

QImage Gradient::toImage(int width, int height) const
{
    width = qMax(1, width);
    height = qMax(1, height);

    QImage row(width, 1, QImage::Format_ARGB32);
    QRgb *line = reinterpret_cast<QRgb*>(row.bits());
    for(int x = 0; x < width; ++x)
    {
        const double t = width > 1 ? double(x) / double(width - 1) : 0.0;
        const QColor c = colorAt(t);
        line[x] = qRgba(c.red(), c.green(), c.blue(), c.alpha());
    }

    if(height == 1)
        return row;

    QImage img(width, height, QImage::Format_ARGB32);
    for(int y = 0; y < height; ++y)
        memcpy(img.scanLine(y), row.constBits(), row.bytesPerLine());
    return img;
}

void Gradient::readFromJson(const QJsonObject &t_json)
{
    const QJsonArray arr = t_json.value("stops").toArray();
    QVector<GradientStop> stops;
    stops.reserve(arr.size());
    for(const auto &v : arr)
    {
        const QJsonObject obj = v.toObject();
        GradientStop stop;
        stop.position = obj.value("position").toDouble();
        stop.color = jsonToColor(obj.value("color").toObject());
        stops.append(stop);
    }
    if(!stops.isEmpty())
        setStops(stops);
}

void Gradient::writeToJson(QJsonObject &t_json) const
{
    QJsonArray arr;
    for(const GradientStop &stop : m_stops)
    {
        QJsonObject obj;
        obj.insert("position", stop.position);
        obj.insert("color", colorToJson(stop.color));
        arr.append(obj);
    }
    t_json.insert("stops", arr);
}

} // namespace photon
