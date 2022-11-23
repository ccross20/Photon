#ifndef PHOTON_CUBEMESH_H
#define PHOTON_CUBEMESH_H
#include "abstractmesh.h"

namespace photon {

class CubeMesh : public AbstractMesh
{
public:
    CubeMesh();

    void setXSize(double value);
    void setYSize(double value);
    void setZSize(double value);

    double xSize() const{return m_size.x();}
    double ySize() const{return m_size.y();}
    double zSize() const{return m_size.z();}

    void rebuild(QOpenGLContext *context) override;

private:
    QVector3D m_size;
};

} // namespace photon

#endif // PHOTON_CUBEMESH_H
