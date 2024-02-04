#ifndef PHOTON_ALIGNTOARROWCLIP_H
#define PHOTON_ALIGNTOARROWCLIP_H

#include <QWidget>
#include "sequence/fixtureclipeffect.h"
#include "scene/scenearrow.h"

namespace photon {

class AlignToArrowClip : public FixtureClipEffect
{
public:
    AlignToArrowClip();

    void init() override;
    void evaluate(FixtureClipEffectEvaluationContext &) override;

    static ClipEffectInformation info();

private:
    SceneArrow *m_arrow;

};

} // namespace photon

#endif // PHOTON_ALIGNTOARROWCLIP_H
