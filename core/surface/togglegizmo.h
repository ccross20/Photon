#ifndef TOGGLEGIZMO_H
#define TOGGLEGIZMO_H
#include <QColor>
#include "surfacegizmo.h"

namespace photon {

// A persistent on/off switch, rendered as a sliding toggle. Unlike ButtonGizmo
// (a momentary/checkable button), the switch's value is a plain persisted
// property rather than runtime press state.
class PHOTONCORE_EXPORT ToggleGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;

    ToggleGizmo();

    QString text() const;
    void setText(const QString &);

    bool value() const;
    void setValue(bool);

    QColor offColor() const;
    void setOffColor(const QColor &);

    QColor onColor() const;
    void setOnColor(const QColor &);

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;

    void setActive(bool) override;
    bool isActive() const override;
};

} // namespace photon

#endif // TOGGLEGIZMO_H
