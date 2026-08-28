#ifndef PHOTON_SCENEAXIS_H
#define PHOTON_SCENEAXIS_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

class SceneAxis;

class SceneAxisEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneAxisEditorWidget(SceneAxis *, QWidget *parent = nullptr);
    ~SceneAxisEditorWidget();

private slots:
    void setName(const QString &name);
    void setSize(float);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);

private:
    class Impl;
    Impl *m_impl;
};

// A flat square plane (side length = size()) lying in the local XY plane,
// with an arrow through its center along local +Z marking the "positive"
// side - e.g. a mirror/reflection plane, or "audience-facing" reference.
// Color and visibility mode come from SceneHelperObject.
class PHOTONCORE_EXPORT SceneAxis : public SceneHelperObject
{
    Q_OBJECT
public:
    SceneAxis();
    ~SceneAxis();

    void setSize(float);
    float size() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEAXIS_H
