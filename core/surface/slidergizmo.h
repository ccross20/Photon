#ifndef SLIDERGIZMO_H
#define SLIDERGIZMO_H
#include <QColor>
#include "surfacegizmo.h"

namespace photon {

class PHOTONCORE_EXPORT SliderGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;

    SliderGizmo();

    QString text() const;
    void setText(const QString &);

    double minValue() const;
    void setMinValue(double);

    double maxValue() const;
    void setMaxValue(double);

    double value() const;
    void setValue(double);

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;

    SurfaceGizmoWidget *createWidget(SurfaceMode mode) override;
};

} // namespace photon

#endif // SLIDERGIZMO_H
