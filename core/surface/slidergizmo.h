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

    SurfaceGizmoWidget *createWidget(SurfaceMode mode) override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    QString m_text = "Slider";
    double m_min = 0;
    double m_max = 1;
    double m_value = 0;

};

} // namespace photon

#endif // SLIDERGIZMO_H
