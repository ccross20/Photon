#ifndef PHOTON_UTILS_H
#define PHOTON_UTILS_H

#include <QColor>

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


class Utils
{
public:
    Utils();
};

} // namespace photon

#endif // PHOTON_UTILS_H
