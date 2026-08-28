#ifndef PHOTON_SCENEBOUNDARYOVAL_H
#define PHOTON_SCENEBOUNDARYOVAL_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

class SceneBoundaryOval;

class SceneBoundaryOvalEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneBoundaryOvalEditorWidget(SceneBoundaryOval *, QWidget *parent = nullptr);
    ~SceneBoundaryOvalEditorWidget();

private slots:
    void setName(const QString &name);
    void setWidth(double);
    void setHeight(double);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);

private:
    class Impl;
    Impl *m_impl;
};

// A 2D oval (ellipse) outline in the local XY plane, for marking a floor
// area - the rounded counterpart to SceneBoundaryRectangle. Color and
// visibility mode come from SceneHelperObject.
class PHOTONCORE_EXPORT SceneBoundaryOval : public SceneHelperObject
{
    Q_OBJECT
public:
    SceneBoundaryOval();
    ~SceneBoundaryOval();

    void setWidth(float);
    void setHeight(float);

    float width() const;
    float height() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEBOUNDARYOVAL_H
