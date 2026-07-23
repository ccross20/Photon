#ifndef XYPADGIZMO_H
#define XYPADGIZMO_H
#include "surfacegizmo.h"

namespace photon {

// A two-axis pad — conceptually a slider with independent X and Y axes,
// each with its own min/max range, published as two separate value-bus
// outputs so they can drive different destinations.
class PHOTONCORE_EXPORT XYPadGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;

    XYPadGizmo();

    QString text() const;
    void setText(const QString &);

    double minX() const;
    void setMinX(double);
    double maxX() const;
    void setMaxX(double);
    double valueX() const;
    void setValueX(double);

    double minY() const;
    void setMinY(double);
    double maxY() const;
    void setMaxY(double);
    double valueY() const;
    void setValueY(double);

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;
};

} // namespace photon

#endif // XYPADGIZMO_H
