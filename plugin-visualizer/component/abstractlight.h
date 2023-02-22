#ifndef PHOTON_ABSTRACTLIGHT_H
#define PHOTON_ABSTRACTLIGHT_H

#include <QColor>
#include "abstractcomponent.h"

namespace photon {

class AbstractLight : public AbstractComponent
{
public:

    enum LightType{
        LightInfinite,
        LightSpot
    };


    AbstractLight(LightType t_type);
    virtual ~AbstractLight();

    LightType type() const{return m_type;}
    void setColor(QColor);
    QColor color() const {return m_color;}

    void setRange(double);
    double range() const {return m_range;}

    void setBrightness(double);
    double brightness() const {return m_brightness;}

    double constant() const {return m_constant;}
    double linear() const {return m_linear;}
    double quadratic() const {return m_quadratic;}

    QVector3D globalDirection() const;
    QVector3D globalUpVector() const;
    QVector3D globalPosition() const;

private:
    QColor m_color;
    double m_brightness = 1.0;
    double m_range = 100.0;
    double m_constant = 1.0;
    double m_linear = .045;
    double m_quadratic = .0075;
    LightType m_type;
};

} // namespace photon

#endif // PHOTON_ABSTRACTLIGHT_H
