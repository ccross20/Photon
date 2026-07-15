#ifndef TOGGLEGIZMO_H
#define TOGGLEGIZMO_H
#include <QDateTime>
#include <QColor>
#include "surfacegizmo.h"

namespace photon {

class PHOTONCORE_EXPORT ToggleGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;
    const static QByteArray GizmoToggleOnId;
    const static QByteArray GizmoToggleOffId;

    ToggleGizmo();

    bool isRadio() const;
    void setIsRadio(bool);
    bool isPressed() const;
    void setIsPressed(bool);
    bool isSticky() const;
    void setIsSticky(bool);

    QString text() const;
    void setText(const QString &);

    QColor offColor() const;
    void setOffColor(const QColor &);

    QColor onColor() const;
    void setOnColor(const QColor &);

    double pressTime() const;
    double timeSincePress() const;

    QVector<GizmoOutput> outputs() const override;
    QVariant outputValue(const QByteArray &portId) const override;

    void setActive(bool) override;
    bool isActive() const override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    // Runtime interaction state (not part of the property model).
    double m_pressTime = 0;
    bool m_isPressed = false;
};

} // namespace photon

#endif // TOGGLEGIZMO_H
