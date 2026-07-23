#ifndef DIALGIZMO_H
#define DIALGIZMO_H
#include "surfacegizmo.h"

namespace photon {

// A rotary knob. Bounded dials sweep a fixed arc mapped to [min,max]; a
// continuous ("infinite") dial spins freely with no rotational limit and
// accumulates its value directly from drag rotation rather than mapping an
// absolute angle to a clamped range.
class PHOTONCORE_EXPORT DialGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;

    DialGizmo();

    QString text() const;
    void setText(const QString &);

    double minValue() const;
    void setMinValue(double);

    double maxValue() const;
    void setMaxValue(double);

    double value() const;
    void setValue(double);

    bool isContinuous() const;
    void setIsContinuous(bool);

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;
};

} // namespace photon

#endif // DIALGIZMO_H
