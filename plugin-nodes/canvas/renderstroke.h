#ifndef PHOTON_RENDERSTROKE_H
#define PHOTON_RENDERSTROKE_H

#include "model/node.h"
#include "graph/parameter/canvasparameter.h"
#include "graph/parameter/pathparameter.h"
#include "graph/parameter/colorparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class RenderStroke : public keira::Node
{
public:
    RenderStroke();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    CanvasParameter *m_canvasInputParam;
    CanvasParameter *m_canvasOutputParam;
    PathParameter *m_pathInputParam;
    keira::DecimalParameter *m_thicknessParam;
    keira::DecimalParameter *m_alphaParam;
    ColorParameter *m_colorParam;
};

} // namespace photon


#endif // PHOTON_RENDERSTROKE_H
