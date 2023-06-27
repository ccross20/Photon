#ifndef PHOTON_DRAWELLIPSE_H
#define PHOTON_DRAWELLIPSE_H

#include "model/node.h"
#include "graph/parameter/pathparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class DrawEllipse : public keira::Node
{
public:
    DrawEllipse();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_widthParam;
    keira::DecimalParameter *m_heightParam;
    PathParameter *m_pathOutputParam;

};

} // namespace photon

#endif // PHOTON_DRAWELLIPSE_H
