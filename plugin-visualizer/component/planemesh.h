#ifndef PHOTON_PLANEMESH_H
#define PHOTON_PLANEMESH_H
#include "abstractmesh.h"

namespace photon {

class PlaneMesh : public AbstractMesh
{
public:
    PlaneMesh();

    void setXSize(double value);
    void setYSize(double value);

    double xSize() const{return m_size.x();}
    double ySize() const{return m_size.y();}

    void rebuild(QOpenGLContext *context) override;

private:
    QVector2D m_size;
};

} // namespace photon


#endif // PHOTON_PLANEMESH_H
