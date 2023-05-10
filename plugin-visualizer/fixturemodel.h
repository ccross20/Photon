#ifndef PHOTON_FIXTUREMODEL_H
#define PHOTON_FIXTUREMODEL_H
#include "sceneobjectmodel.h"

namespace photon {

class FixtureModel : public SceneObjectModel
{
public:
    FixtureModel(SceneObject *sceneObj, QObject *parent = nullptr);


    void preDraw(double elapsed) override;
    void updateFromDMX(const DMXMatrix &, const double elapsed) override;
private:
    void recursiveAddModifiers(Entity *);
    QVector<ModelModifier*> m_modifiers;
};

} // namespace photon

#endif // PHOTON_FIXTUREMODEL_H
