#ifndef PHOTON_COLORCAPABILITY_H
#define PHOTON_COLORCAPABILITY_H

#include <QMap>
#include "fixturecapability.h"
#include "colorintensitycapability.h"
#include "fixturecolorcalibration.h"

namespace photon {

class PHOTONCORE_EXPORT ColorCapability : public FixtureCapability
{
public:
    ColorCapability(const QVector<ColorIntensityCapability*> &);
    ~ColorCapability();

    void setColor(const QColor &, DMXMatrix &t_matrix, double t_blend = 1.0) const;
    QColor getColor(const DMXMatrix &t_matrix) const;

    bool hasWhite() const;
    bool hasAmber() const;
    bool hasLime() const;
    bool isCMY() const;

    // The color-mixing channel types this capability actually drives - what
    // the color calibration dialog uses to decide which sliders to draw.
    QVector<CapabilityType> channelTypes() const;

    // Raw passthrough: stamps each percent directly onto its matching
    // ColorIntensityCapability channel, with no color math. Used both by the
    // live calibration preview and by setColor() once calibrated.
    void setChannelPercents(const QMap<CapabilityType, double> &, DMXMatrix &t_matrix, double t_blend = 1.0) const;

    void setCalibration(const FixtureColorCalibration &);
    bool hasCalibration() const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORCAPABILITY_H
