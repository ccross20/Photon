#include <QElapsedTimer>
#include "lampmodifier.h"
#include "component/spotlight.h"
#include "fixture/fixture.h"
#include "fixture/capability/anglecapability.h"
#include "fixture/capability/dimmercapability.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/capability/shutterstrobecapability.h"

namespace photon {

class Strober
{
public:
    double Update(ShutterStrobeCapability *t_capability, const DMXMatrix &t_matrix, double t_elapsed)
    {
        /*
        Shutter_Open,
        Shutter_Closed,
        Shutter_Strobe,
        Shutter_Pulse,
        Shutter_RampUp,
        Shutter_RampDown,
        Shutter_RampUpDown,
        Shutter_Lightning,
        Shutter_Spikes,
        Shutter_Burst
        */
        double min = 30.0/1000.0;
        double max = 1;

        auto effect = t_capability->shutterEffect();

        if(effect != m_shutterEffect)
            m_timer.restart();

        m_shutterEffect = effect;
        double targetTime = ((max - min) * t_capability->getSpeedPercent(t_matrix)) + min;


        //qDebug() << "target" << targetTime << "Elapsed:" << t_elapsed << "Since last:" << m_sinceLastPulse << "Mode" << m_shutterEffect;

        switch (m_shutterEffect) {
            default:
            case ShutterStrobeCapability::Shutter_Open:
                m_sinceLastPulse = 0.0;
                return 1.0;
            case ShutterStrobeCapability::Shutter_Closed:
                m_sinceLastPulse = 0.0;
                return 0.0;
            case ShutterStrobeCapability::Shutter_Strobe:
            {
                if(m_sinceLastPulse > targetTime)
                {
                    m_sinceLastPulse =  (m_timer.elapsed()/1000.0) / targetTime - floor((m_timer.elapsed()/1000.0) / targetTime);
                    return 1.0;
                }
                m_sinceLastPulse += t_elapsed;
                return 0.0;
            }

                break;
        }

        return 1.0;
    }


private:
    ShutterStrobeCapability::ShutterEffect m_shutterEffect = ShutterStrobeCapability::Shutter_Open;
    double m_sinceLastPulse = 0;
    QElapsedTimer m_timer;
};

LampModifier::LampModifier(SceneObjectModel *t_model,  Entity *t_entity) : ModelModifier(t_model, t_entity)
{
    m_strober = new Strober;

    auto focuses = fixture()->findCapability(Capability_Focus);
    auto zooms = fixture()->findCapability(Capability_Zoom);
    auto dimmers = fixture()->findCapability(Capability_Dimmer);
    auto colors = fixture()->findCapability(Capability_Color);
    m_strobes = fixture()->findCapability<ShutterStrobeCapability*>();

    if(focuses.length() > 0)
    {
        m_focusCapability = static_cast<AngleCapability*>(focuses[0]);
    }

    if(zooms.length() > 0)
    {
        m_zoomCapability = static_cast<AngleCapability*>(zooms[0]);
    }

    if(dimmers.length() > 0)
    {
        m_dimmerCapability = static_cast<DimmerCapability*>(dimmers[0]);
    }

    if(colors.length() > 0)
    {
        m_colorCapability = static_cast<ColorCapability*>(colors[0]);
    }

    m_light = entity()->findComponent<SpotLight*>();

}

LampModifier::~LampModifier()
{
    delete m_strober;
}

void LampModifier::updateModel(const DMXMatrix &t_matrix, const double t_elapsed)
{
    if(!m_light)
        return;
    if(m_focusCapability)
    {
        float focusPercent = m_focusCapability->getAnglePercent(t_matrix);
        m_light->setHardness(focusPercent);
    }

    if(m_zoomCapability)
    {
        float zoomPercent = m_zoomCapability->getAnglePercent(t_matrix);

        float angle = (zoomPercent * (fixture()->physical().lensMaximum - fixture()->physical().lensMinimum)) + fixture()->physical().lensMinimum;

        m_light->setAngle(angle);
    }

    if(m_dimmerCapability)
    {
        float dimmerPercent = m_dimmerCapability->getPercent(t_matrix);
        m_light->setBrightness(dimmerPercent);
    }

    if(m_colorCapability)
    {
        QColor colorVal = m_colorCapability->getColor(t_matrix);
        m_light->setColor(colorVal.toRgb());
    }

    if(!m_strobes.isEmpty())
    {
        for(auto strobeCapability : m_strobes)
        {
            if(strobeCapability->isValid(t_matrix))
            {

                double val = m_strober->Update(strobeCapability, t_matrix, t_elapsed);

                m_light->setBrightness(m_light->brightness() * val);
            }
        }
    }


}

} // namespace photon
