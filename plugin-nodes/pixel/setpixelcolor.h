#ifndef SETPIXELCOLOR_H
#define SETPIXELCOLOR_H


#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "graph/parameter/colorparameter.h"


namespace photon {

class SetPixelColor : public keira::Node
{
public:
    SetPixelColor();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

protected:
    void parameterWasModified(keira::Parameter*) override;

private:
    ColorParameter *m_colorParam;
    keira::IntegerParameter *m_priorityParam;
    keira::DecimalParameter *m_blendParam;
    keira::IntegerParameter *m_pixelParam;
};

} // namespace photon

#endif // SETPIXELCOLOR_H
