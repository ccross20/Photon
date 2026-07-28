#ifndef PHOTON_GRADIENT_H
#define PHOTON_GRADIENT_H

#include <QColor>
#include <QVector>
#include <QMetaType>
#include "photon-global.h"

class QImage;
class QJsonObject;

namespace photon {

struct GradientStop
{
    double position = 0.0;   // normalised 0..1
    QColor color = Qt::black;

    bool operator==(const GradientStop &o) const { return qFuzzyCompare(position + 1.0, o.position + 1.0) && color == o.color; }
    bool operator!=(const GradientStop &o) const { return !(*this == o); }
};

// A linear list of colour stops with straight (non-premultiplied) RGBA
// interpolation between neighbours. Stops are always kept sorted by position.
class PHOTONCORE_EXPORT Gradient
{
public:
    Gradient();                                      // default black -> white
    explicit Gradient(const QVector<GradientStop> &stops);

    const QVector<GradientStop> &stops() const { return m_stops; }
    void setStops(const QVector<GradientStop> &stops);

    int count() const { return m_stops.size(); }
    const GradientStop &stopAt(int i) const { return m_stops.at(i); }

    // Insert a stop keeping the list sorted; returns the new stop's index.
    int addStop(double position, const QColor &color);
    void removeStop(int index);
    void setStopColor(int index, const QColor &color);
    // Move a stop's position, re-sorting if needed; returns the stop's new index.
    int setStopPosition(int index, double position);

    // Interpolated colour at t in [0,1] (clamped outside the stop range).
    QColor colorAt(double t) const;

    // A copy with the stop order mirrored (position -> 1 - position).
    Gradient reversed() const;

    // A copy with stop positions transformed by pos*scale + offset. In "repeat"
    // mode the gradient is treated as periodic (period 1) so it cycles/tiles;
    // otherwise transformed positions are clamped into [0,1].
    Gradient remapped(double offset, double scale, bool repeat) const;

    // Per-channel blend of two gradients at t across the union of their stops.
    static Gradient mixed(const Gradient &a, const Gradient &b, double factor);

    // Horizontal gradient rendered to an image; handy for preview swatches and
    // for uploading to a shader as a 1-D lookup texture.
    QImage toImage(int width, int height = 1) const;

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

    bool operator==(const Gradient &o) const { return m_stops == o.m_stops; }
    bool operator!=(const Gradient &o) const { return !(*this == o); }

private:
    void sort();

    QVector<GradientStop> m_stops;
};

} // namespace photon

Q_DECLARE_METATYPE(photon::Gradient)

#endif // PHOTON_GRADIENT_H
