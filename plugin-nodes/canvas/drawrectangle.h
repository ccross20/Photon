#ifndef PHOTON_DRAWRECTANGLE_H
#define PHOTON_DRAWRECTANGLE_H

#include "model/node.h"
#include "graph/parameter/canvasparameter.h"
#include "graph/parameter/colorparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class DrawRectangle : public keira::Node
{
public:
    DrawRectangle();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    CanvasParameter *m_canvasInputParam;
    CanvasParameter *m_canvasOutputParam;
    keira::DecimalParameter *m_widthParam;
    keira::DecimalParameter *m_heightParam;
    keira::DecimalParameter *m_alphaParam;
    ColorParameter *m_colorParam;
};

} // namespace photon

#endif // PHOTON_DRAWRECTANGLE_H
