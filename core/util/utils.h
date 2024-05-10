#ifndef PHOTON_UTILS_H
#define PHOTON_UTILS_H

#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
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

static QJsonObject colorToJson(const QColor &t_color)
{
    QJsonObject obj;
    obj.insert("spec", t_color.spec());

    switch (t_color.spec()) {
    case QColor::Rgb:
        obj.insert("r", t_color.redF());
        obj.insert("g", t_color.greenF());
        obj.insert("b", t_color.blueF());
        break;
    case QColor::Hsv:
        obj.insert("h", t_color.hueF());
        obj.insert("s", t_color.saturationF());
        obj.insert("v", t_color.valueF());
        break;
    case QColor::Hsl:
        obj.insert("h", t_color.hueF());
        obj.insert("s", t_color.saturationF());
        obj.insert("l", t_color.lightnessF());
        break;
    case QColor::Cmyk:
        obj.insert("c", t_color.cyanF());
        obj.insert("m", t_color.magentaF());
        obj.insert("y", t_color.yellowF());
        obj.insert("k", t_color.blackF());
        break;
    default:
        break;
    }

    obj.insert("a", t_color.alphaF());


    return obj;
}

static QColor jsonToColor(const QJsonObject &t_obj)
{
    int spec = t_obj.value("spec").toInt();

    switch (spec) {
    case QColor::Rgb:
        return QColor::fromRgbF(t_obj.value("r").toDouble(), t_obj.value("g").toDouble(),t_obj.value("b").toDouble(),t_obj.value("a").toDouble());
    case QColor::Hsv:
        return QColor::fromHsvF(t_obj.value("h").toDouble(), t_obj.value("s").toDouble(),t_obj.value("v").toDouble(),t_obj.value("a").toDouble());
    case QColor::Hsl:
        return QColor::fromHslF(t_obj.value("h").toDouble(), t_obj.value("s").toDouble(),t_obj.value("l").toDouble(),t_obj.value("a").toDouble());
    case QColor::Cmyk:
        return QColor::fromCmykF(t_obj.value("c").toDouble(), t_obj.value("m").toDouble(),t_obj.value("y").toDouble(),t_obj.value("k").toDouble(),t_obj.value("a").toDouble());
    default:
        break;
    }

    return QColor();
}

static QJsonObject pointFToJson(const QPointF &t_pt)
{
    QJsonObject obj;
    obj.insert("x", t_pt.x());
    obj.insert("y", t_pt.y());
    return obj;
}


static QJsonArray stringListToJson(const QStringList &t_list)
{
    QJsonArray array;
    for(const auto &str : t_list)
        array << str;
    return array;
}

static QStringList jsonToStringList(const QJsonArray &t_array)
{
    QStringList list;
    for(const auto &str : t_array)
        list << str.toString();
    return list;
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
