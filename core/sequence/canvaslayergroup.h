#ifndef PHOTON_CANVASLAYERGROUP_H
#define PHOTON_CANVASLAYERGROUP_H
#include "layergroup.h"

namespace photon {

class CanvasLayerGroup : public LayerGroup
{
public:
    CanvasLayerGroup(Canvas *, const QString &name);
    ~CanvasLayerGroup();

    void setCanvas(Canvas *);
    Canvas *canvas() const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASLAYERGROUP_H
