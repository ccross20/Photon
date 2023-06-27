#ifndef PHOTON_COLORFROMHSV_H
#define PHOTON_COLORFROMHSV_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "graph/parameter/colorparameter.h"

namespace photon {

class ColorFromHSV : public keira::Node
{
public:
    ColorFromHSV();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::DecimalParameter *hueParam;
    keira::DecimalParameter *satParam;
    keira::DecimalParameter *valueParam;
    ColorParameter *outputParam;
};

} // namespace photon

#endif // PHOTON_COLORFROMHSV_H
