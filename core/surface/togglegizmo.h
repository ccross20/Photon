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

    SurfaceGizmoWidget *createWidget(SurfaceMode mode) override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    double m_pressTime;
    bool m_isPressed = false;
    bool m_isSticky = false;
    bool m_isRadio = false;
    QString m_text = "Toggle";
    QColor m_offColor = Qt::white;
    QColor m_onColor = Qt::green;

};

} // namespace photon

#endif // TOGGLEGIZMO_H
