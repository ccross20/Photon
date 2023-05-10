#include "trussmodel.h"
#include "geometry/trussgeometry.h"
#include "component/materialcomponent.h"
#include "entity.h"
#include "scene/truss.h"

namespace photon {

TrussModel::TrussModel(SceneObject *t_sceneObj, QObject *t_parent) : SceneObjectModel(t_sceneObj, t_parent)
{
    MaterialComponent *simpleMaterial = new MaterialComponent;
    simpleMaterial->setDiffuseColor(QColor(255,255,255));
    simpleMaterial->setFragmentSource(":/resources/shader/basic_lighting.frag");
    simpleMaterial->setVertexSource(":/resources/shader/basic_lighting.vert");



    m_truss = new TrussGeometry;
    metadataUpdated();

    entity()->addComponent(m_truss);
    entity()->addComponent(simpleMaterial);
}

void TrussModel::metadataUpdated()
{
    Truss *truss = static_cast<Truss*>(sceneObject());
    m_truss->setSegmentLength(truss->segmentLength());
    m_truss->setLength(truss->length());
    m_truss->setRadius(truss->radius());
    m_truss->setOffset(truss->offset());
    m_truss->setBeams(truss->beams());
    m_truss->setAngle(truss->angle());
}

} // namespace photon
