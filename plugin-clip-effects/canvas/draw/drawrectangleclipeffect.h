#ifndef PHOTON_DRAWRECTANGLECLIPEFFECT_H
#define PHOTON_DRAWRECTANGLECLIPEFFECT_H

#include <QWidget>
#include "sequence/canvasclipeffect.h"

namespace photon {

class DrawRectangleClipEffect : public CanvasClipEffect
{
public:
    DrawRectangleClipEffect();

    void init() override;
    void evaluate(CanvasClipEffectEvaluationContext &) const override;
    static ClipEffectInformation info();

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    QColor fillColor;
    QColor strokeColor;
    QPointF centerPt;
    QPointF position;
    QPointF scale;
    double rotation;
    double strokeWidth;
};

} // namespace photon

#endif // PHOTON_DRAWRECTANGLECLIPEFFECT_H
