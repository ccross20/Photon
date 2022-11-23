#ifndef PHOTON_VISUALIZERSCENE_H
#define PHOTON_VISUALIZERSCENE_H

#include <QObject>
#include <Qt3DCore/qentity.h>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DRender>

namespace photon {

class VisualizerScene : public QObject
{
    Q_OBJECT
public:
    explicit VisualizerScene(Qt3DCore::QEntity *rootEntity);
    ~VisualizerScene();

signals:

private:
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DExtras::QPlaneMesh *m_torus;
    Qt3DCore::QEntity *m_torusEntity;
    Qt3DRender::QPointLight *m_light;
    Qt3DRender::QPaintedTextureImage *m_gobo;
};

} // namespace photon

#endif // PHOTON_VISUALIZERSCENE_H
