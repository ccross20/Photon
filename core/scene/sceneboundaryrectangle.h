#ifndef PHOTON_SCENEBOUNDARYRECTANGLE_H
#define PHOTON_SCENEBOUNDARYRECTANGLE_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

class SceneBoundaryRectangle;

class SceneBoundaryRectangleEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneBoundaryRectangleEditorWidget(SceneBoundaryRectangle *, QWidget *parent = nullptr);
    ~SceneBoundaryRectangleEditorWidget();

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

// A 2D rectangle outline in the local XY plane, for marking a floor area
// (e.g. a dance floor). Color and visibility mode come from SceneHelperObject.
class PHOTONCORE_EXPORT SceneBoundaryRectangle : public SceneHelperObject
{
    Q_OBJECT
public:
    SceneBoundaryRectangle();
    ~SceneBoundaryRectangle();

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

#endif // PHOTON_SCENEBOUNDARYRECTANGLE_H
