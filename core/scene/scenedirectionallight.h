#ifndef PHOTON_SCENEDIRECTIONALLIGHT_H
#define PHOTON_SCENEDIRECTIONALLIGHT_H

#include <QColor>
#include "photon-global.h"
#include "scene/sceneobject.h"

namespace photon {

// Parallel light from one direction (sun, a general key light), so surfaces
// at different angles shade differently. It shines along the object's local
// -Y - aim it with the rotate gizmo; position only places the arrow drawn in
// the visualizer. Lights walls, floors, boxes, fixtures and truss; several
// add together (the visualizer uses up to MaximumLights of them).
class PHOTONCORE_EXPORT SceneDirectionalLight : public SceneObject
{
    Q_OBJECT
public:
    static constexpr int MaximumLights = 4;

    SceneDirectionalLight();
    ~SceneDirectionalLight();

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

#endif // PHOTON_SCENEDIRECTIONALLIGHT_H
