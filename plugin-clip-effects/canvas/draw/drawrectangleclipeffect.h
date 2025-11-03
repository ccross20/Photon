#ifndef PHOTON_DRAWRECTANGLECLIPEFFECT_H
#define PHOTON_DRAWRECTANGLECLIPEFFECT_H

#include <QWidget>
#include "sequence/canvaseffect.h"

namespace photon {

class DrawRectangleClipEffect : public CanvasEffect
{
public:
    DrawRectangleClipEffect();

    void init() override;
    void evaluate(CanvasEffectEvaluationContext &) override;
    static ClipEffectInformation info();

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;
};

} // namespace photon

#endif // PHOTON_DRAWRECTANGLECLIPEFFECT_H
