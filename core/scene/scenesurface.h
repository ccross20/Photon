#ifndef PHOTON_SCENESURFACE_H
#define PHOTON_SCENESURFACE_H

#include <QWidget>
#include <QColor>
#include "photon-global.h"
#include "scene/sceneobject.h"

namespace photon {

class SceneSurface;

class SceneSurfaceEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneSurfaceEditorWidget(SceneSurface *, QWidget *parent = nullptr);
    ~SceneSurfaceEditorWidget();

private slots:
    void setName(const QString &name);
    void setWidth(double);
    void setHeight(double);
    void chooseColor();
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);
    void refreshTransform();

private:
    class Impl;
    Impl *m_impl;
};

// A flat rectangular surface (wall / floor) that fixtures can light. The plane
// lies in the local XY plane, centered at the origin, with its normal along +Z.
// Orient it with the transform gizmo (a floor is just a surface rotated flat).
class PHOTONCORE_EXPORT SceneSurface : public SceneObject
{
    Q_OBJECT
public:
    SceneSurface();
    ~SceneSurface();

    void setSurfaceWidth(float);
    void setSurfaceHeight(float);
    void setColor(const QColor &);

    float surfaceWidth() const;
    float surfaceHeight() const;
    QColor color() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENESURFACE_H
