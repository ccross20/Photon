#ifndef PALETTEGIZMOCHANNELEFFECT_H
#define PALETTEGIZMOCHANNELEFFECT_H

#include "surface/gizmochanneleffect.h"

namespace photon {

class PaletteGizmoChannelEffect;

class PaletteGizmoChannelEffectEditor : public GizmoChannelEffectEditor
{
    Q_OBJECT
public:
    PaletteGizmoChannelEffectEditor(PaletteGizmoChannelEffect *);

};


class PaletteGizmoChannelEffect : public GizmoChannelEffect
{
public:
    PaletteGizmoChannelEffect();

    float * process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;



    static EffectInformation info();
};

} // namespace photon

#endif // PALETTEGIZMOCHANNELEFFECT_H
