#ifndef PHOTON_TRUSSGEOMETRY_H
#define PHOTON_TRUSSGEOMETRY_H
#include <Qt3DCore>

using namespace Qt3DCore;

namespace photon {

class TrussGeometry: public QGeometry
{
public:
    TrussGeometry(QNode *parent = nullptr);
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


private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TRUSSGEOMETRY_H
