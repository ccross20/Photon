#ifndef PHOTON_LAMPMODIFIER_H
#define PHOTON_LAMPMODIFIER_H
#include "modelmodifier.h"

namespace photon {

class AngleCapability;
class SpotLight;
class DimmerCapability;
class ShutterStrobeCapability;
class ColorCapability;
class Strober;

class LampModifier : public ModelModifier
{


public:
    LampModifier(SceneObjectModel *model,  Entity *entity);
    ~LampModifier();


    void updateModel(const DMXMatrix &t_matrix, const double t_elapsed) override;

private:
    Strober *m_strober = nullptr;
    SpotLight *m_light = nullptr;
    AngleCapability *m_focusCapability = nullptr;
    AngleCapability *m_zoomCapability = nullptr;
    DimmerCapability *m_dimmerCapability = nullptr;
    ColorCapability *m_colorCapability = nullptr;
    QVector<ShutterStrobeCapability*> m_strobes;
};

} // namespace photon

#endif // PHOTON_LAMPMODIFIER_H
