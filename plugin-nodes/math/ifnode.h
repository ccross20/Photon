#ifndef PHOTON_IFNODE_H
#define PHOTON_IFNODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

// Compares two numbers and outputs one of two values depending on the answer,
// so it doubles as a comparison and as a switch between two numbers. The
// defaults of 1 and 0 make the output read as a plain boolean - and a boolean
// input accepts it directly, treating anything above 0 as true.
class IfNode : public keira::Node
{
public:
    enum Mode
    {
        ModeGreater,
        ModeLess,
        ModeGreaterOrEqual,
        ModeLessOrEqual,
        ModeNotEqual,
        ModeEqual,
        ModeEven,
        ModeOdd,
        ModeDivisible,
    };

    IfNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_aParam;
    keira::DecimalParameter *m_bParam;
    keira::OptionParameter *m_modeParam;
    keira::DecimalParameter *m_trueParam;
    keira::DecimalParameter *m_falseParam;
    keira::DecimalParameter *m_resultParam;
};

} // namespace photon

#endif // PHOTON_IFNODE_H
