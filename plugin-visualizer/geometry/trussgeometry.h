#ifndef PHOTON_TRUSSGEOMETRY_H
#define PHOTON_TRUSSGEOMETRY_H
#include "component/abstractmesh.h"

namespace photon {

class TrussGeometry: public AbstractMesh
{
public:
    TrussGeometry();
    ~TrussGeometry();


    void setBeams(uint);
    void setSegmentLength(float);
    void setRadius(float);
    void setOffset(float);
    void setLength(float);
    void setAngle(float t_degrees);

    uint beams() const;
    float radius() const;
    float offset() const;
    float segmentLength() const;
    float length() const;
    float angle() const;

    void rebuild(QOpenGLContext *context) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TRUSSGEOMETRY_H
