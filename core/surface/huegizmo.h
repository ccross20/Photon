#ifndef HUEGIZMO_H
#define HUEGIZMO_H
#include "surfacegizmo.h"

namespace photon {

// A color wheel restricted to hue selection — dragging around the ring picks
// an angle in [0,360). Publishes both the raw hue and a fully-saturated
// QColor derived from it, so it can drive either a numeric or a color input.
class PHOTONCORE_EXPORT HueGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;

    HueGizmo();

    QString text() const;
    void setText(const QString &);

    double hue() const;
    void setHue(double);

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;
};

} // namespace photon

#endif // HUEGIZMO_H
