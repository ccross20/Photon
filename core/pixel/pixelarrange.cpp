#include <QTransform>
#include <QtMath>
#include "pixelarrange.h"

namespace photon {

namespace PixelArrange {

QVector<QPointF> linear(int t_count, double t_length, double t_center, double t_angle)
{
    QVector<QPointF> result;
    if(t_count <= 0)
        return result;

    QTransform t;
    t.translate(t_length * t_center, 0);
    t.rotate(t_angle);
    t.translate(t_length * -t_center, 0);

    result.resize(t_count);
    double delta = t_count > 1 ? t_length / (t_count - 1) : 0;
    double position = 0;
    for(auto it = result.begin(); it != result.end(); ++it)
    {
        *it = t.map(QPointF(position, 0.0));
        position += delta;
    }

    return result;
}

QVector<QPointF> grid(int t_count, int t_rows, int t_cols, double t_width, double t_height)
{
    QVector<QPointF> result;
    if(t_count <= 0 || t_rows <= 0 || t_cols <= 0)
        return result;

    double dx = t_cols > 1 ? t_width / (t_cols - 1) : 0;
    double dy = t_rows > 1 ? t_height / (t_rows - 1) : 0;

    result.reserve(t_count);
    for(int i = 0; i < t_count; ++i)
    {
        int row = i / t_cols;
        int col = i % t_cols;
        result << QPointF(-t_width / 2.0 + col * dx, -t_height / 2.0 + row * dy);
    }

    return result;
}

QVector<QPointF> radial(int t_count, double t_radius)
{
    QVector<QPointF> result;
    if(t_count <= 0)
        return result;

    result.reserve(t_count);
    double step = 2.0 * M_PI / t_count;
    for(int i = 0; i < t_count; ++i)
        result << QPointF(t_radius * std::cos(i * step), t_radius * std::sin(i * step));

    return result;
}

QVector<QPointF> arc(int t_count, double t_radius, double t_startAngle, double t_sweepAngle)
{
    QVector<QPointF> result;
    if(t_count <= 0)
        return result;

    double startRad = qDegreesToRadians(t_startAngle);
    double sweepRad = qDegreesToRadians(t_sweepAngle);
    double step = t_count > 1 ? sweepRad / (t_count - 1) : 0;

    result.reserve(t_count);
    for(int i = 0; i < t_count; ++i)
    {
        double a = startRad + i * step;
        result << QPointF(t_radius * std::cos(a), t_radius * std::sin(a));
    }

    return result;
}

QVector<QPointF> honeycomb(int t_count, int t_rows, int t_cols, double t_spacing)
{
    QVector<QPointF> result;
    if(t_count <= 0 || t_rows <= 0 || t_cols <= 0)
        return result;

    double dx = t_spacing;
    double dy = t_spacing * std::sqrt(3.0) / 2.0;
    double totalWidth = (t_cols - 1) * dx + dx / 2.0;
    double totalHeight = (t_rows - 1) * dy;

    result.reserve(t_count);
    for(int i = 0; i < t_count; ++i)
    {
        int row = i / t_cols;
        int col = i % t_cols;
        double xOffset = (row % 2 != 0) ? dx / 2.0 : 0.0;
        result << QPointF(-totalWidth / 2.0 + col * dx + xOffset, -totalHeight / 2.0 + row * dy);
    }

    return result;
}

QVector<QPointF> beeEye(int t_count, double t_spacing)
{
    QVector<QPointF> result;
    if(t_count <= 0)
        return result;

    // Axial hex directions and ring-walk order, per redblobgames.com's
    // hexagonal grid reference - direction[4] is the ring's starting corner,
    // then each of the 6 sides is walked for `ring` steps to produce exactly
    // 6*ring points forming a closed hexagonal ring.
    static const int directions[6][2] = {
        {1,0}, {1,-1}, {0,-1}, {-1,0}, {-1,1}, {0,1}
    };
    // The standard axial->pixel conversion treats "size" as the hex cell's
    // circumradius, whose adjacent-center distance works out to size*sqrt(3)
    // - not size itself. Divide out that factor so t_spacing means what it
    // means everywhere else in this module: literal adjacent-point distance.
    double size = t_spacing / std::sqrt(3.0);
    auto toCartesian = [size](int q, int r)
    {
        double x = size * (std::sqrt(3.0) * q + std::sqrt(3.0) / 2.0 * r);
        double y = size * (1.5 * r);
        return QPointF(x, y);
    };

    result << QPointF(0, 0);

    int ring = 1;
    while(result.size() < t_count)
    {
        int q = directions[4][0] * ring;
        int r = directions[4][1] * ring;

        for(int side = 0; side < 6 && result.size() < t_count; ++side)
        {
            for(int step = 0; step < ring && result.size() < t_count; ++step)
            {
                result << toCartesian(q, r);
                q += directions[side][0];
                r += directions[side][1];
            }
        }
        ++ring;
    }

    return result;
}

} // namespace PixelArrange

} // namespace photon
