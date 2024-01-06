#ifndef PHOTON_ARROWMODEL_H
#define PHOTON_ARROWMODEL_H

#include "sceneobjectmodel.h"

namespace photon {

class ArrowModel : public SceneObjectModel
{
public:
    ArrowModel(SceneObject *sceneObj, QObject *parent = nullptr);

protected:
    void metadataUpdated() override;
};

} // namespace photon

#endif // PHOTON_ARROWMODEL_H
