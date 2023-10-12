#ifndef PHOTON_UTILS_H
#define PHOTON_UTILS_H

#include <QColor>
#include <QJsonObject>
#include <QPoint>
#include <QPointF>

namespace photon {

static QColor blendColors(const QColor &t_a, const QColor &t_b, double t_t){
    QColor a = t_a.convertTo(QColor::Rgb);
    QColor b = t_b.convertTo(QColor::Rgb);

    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t_t,
                            a.greenF() + (b.greenF() - a.greenF()) * t_t,
                            a.blueF() + (b.blueF() - a.blueF()) * t_t,
                            a.alphaF() + (b.alphaF() - a.alphaF()) * t_t
                            );
}

static QJsonObject pointToJson(const QPoint &t_pt)
{
    QJsonObject obj;
    obj.insert("x", t_pt.x());
    obj.insert("y", t_pt.y());
    return obj;
}

static QJsonObject pointFToJson(const QPointF &t_pt)
{
    QJsonObject obj;
    obj.insert("x", t_pt.x());
    obj.insert("y", t_pt.y());
    return obj;
}

static QPoint jsonToPoint(const QJsonObject &t_obj)
{
    return QPoint{t_obj.value("x").toInt(), t_obj.value("y").toInt()};
}

static QPointF jsonToPointF(const QJsonObject &t_obj)
{
    return QPointF{t_obj.value("x").toDouble(), t_obj.value("y").toDouble()};
}


class Utils
{
public:
    Utils();
};

} // namespace photon

#endif // PHOTON_UTILS_H
