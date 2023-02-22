#ifndef PHOTON_SPOTLIGHT_H
#define PHOTON_SPOTLIGHT_H
#include <QImage>
#include "abstractlight.h"

namespace photon {

class SpotLight : public AbstractLight
{
public:
    SpotLight();
    ~SpotLight();

    void setAngle(double);
    double angle() const {return m_angle;}

    void setHardness(double);
    double hardness() const {return m_hardness;}

    QMatrix4x4 matrix() const;

    void setCookie(const QImage &);
    QImage cookie() {return m_cookie;}

    void setGobo(const QImage &);
    QImage gobo() const {return m_gobo;}

private:
    double m_angle;
    double m_hardness = 1.0;
    QImage m_cookie;
    QImage m_gobo;
};

} // namespace photon

#endif // PHOTON_SPOTLIGHT_H
