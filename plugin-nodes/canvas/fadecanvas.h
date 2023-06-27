#ifndef PHOTON_FADECANVAS_H
#define PHOTON_FADECANVAS_H

#include "model/node.h"
#include "graph/parameter/canvasparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class FadeCanvas : public keira::Node
{
public:
    FadeCanvas();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    CanvasParameter *m_canvasInputParam;
    CanvasParameter *m_canvasOutputParam;
    keira::IntegerParameter *m_delayParam;
};

} // namespace photon

#endif // PHOTON_FADECANVAS_H
