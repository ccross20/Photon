#ifndef PHOTON_GRIDPLANE_H
#define PHOTON_GRIDPLANE_H

#include <Qt3DCore>

namespace photon {

class GridPlane : public Qt3DCore::QEntity
{
public:
    GridPlane(Qt3DCore::QNode *parent = nullptr);
    void init();
    ~GridPlane();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_GRIDPLANE_H
