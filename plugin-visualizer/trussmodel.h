#ifndef PHOTON_TRUSSMODEL_H
#define PHOTON_TRUSSMODEL_H

#include "sceneobjectmodel.h"

namespace photon {

class TrussGeometry;

class TrussModel : public SceneObjectModel
{
public:
    TrussModel(SceneObject *sceneObj, QObject *parent = nullptr);

protected:
    void metadataUpdated() override;

private :
    TrussGeometry *m_truss;
};

} // namespace photon

#endif // PHOTON_TRUSSMODEL_H
