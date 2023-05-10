#ifndef PHOTON_COLORCAPABILITY_H
#define PHOTON_COLORCAPABILITY_H

#include "fixturecapability.h"
#include "colorintensitycapability.h"

namespace photon {

class PHOTONCORE_EXPORT ColorCapability : public FixtureCapability
{
public:
    ColorCapability(const QVector<ColorIntensityCapability*> &);
    ~ColorCapability();

    void setColor(const QColor &, DMXMatrix &t_matrix, double t_blend = 1.0) const;
    QColor getColor(const DMXMatrix &t_matrix) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORCAPABILITY_H
