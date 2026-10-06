#ifndef PHOTON_SCENEAMBIENTLIGHT_H
#define PHOTON_SCENEAMBIENTLIGHT_H

#include <QColor>
#include "photon-global.h"
#include "scene/sceneobject.h"

namespace photon {

// Even, directionless light filling the whole room (house lights, spill from
// screens). The visualizer adds up every visible ambient light in the scene
// - color times intensity - and lights walls, floors, boxes, fixtures and
// truss with the total. Position and rotation have no effect.
class PHOTONCORE_EXPORT SceneAmbientLight : public SceneObject
{
    Q_OBJECT
public:
    SceneAmbientLight();
    ~SceneAmbientLight();

    void setColor(const QColor &);
    void setIntensity(double);   // 0..1

    QColor color() const;
    double intensity() const;

    QWidget *createEditor() override;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCENEAMBIENTLIGHT_H
