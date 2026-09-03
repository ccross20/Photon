#ifndef PHOTON_SCENELINEARFALLOFF_H
#define PHOTON_SCENELINEARFALLOFF_H

#include <QWidget>
#include "photon-global.h"
#include "scene/scenehelperobject.h"

namespace photon {

class SceneLinearFalloff;

class SceneLinearFalloffEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneLinearFalloffEditorWidget(SceneLinearFalloff *, QWidget *parent = nullptr);
    ~SceneLinearFalloffEditorWidget();

private slots:
    void setName(const QString &name);
    void setLength(double);
    void setPosition(const QVector3D &);
    void setRotation(const QVector3D &);

private:
    class Impl;
    Impl *m_impl;
};

// A falloff gradient laid out in space: an arrow along local +Y with a
// perpendicular cross-bar at its start and end, marking where a linear falloff
// begins (full) and ends (zero). Length controls the arrow (and so the falloff
// distance). Color and visibility mode come from SceneHelperObject.
class PHOTONCORE_EXPORT SceneLinearFalloff : public SceneHelperObject
{
    Q_OBJECT
public:
    SceneLinearFalloff();
    ~SceneLinearFalloff();

    void setLength(float);
    float length() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENELINEARFALLOFF_H
