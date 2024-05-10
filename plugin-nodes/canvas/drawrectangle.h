#ifndef PHOTON_DRAWRECTANGLE_H
#define PHOTON_DRAWRECTANGLE_H

#include "model/node.h"
#include "graph/parameter/pathparameter.h"
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
    PathParameter *m_pathOutputParam;
    keira::DecimalParameter *m_widthParam;
    keira::DecimalParameter *m_heightParam;
    keira::DecimalParameter *m_radiusParam;
};

} // namespace photon

#endif // PHOTON_DRAWRECTANGLE_H
