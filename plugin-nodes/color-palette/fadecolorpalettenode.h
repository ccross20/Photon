#ifndef FADECOLORPALETTENODE_H
#define FADECOLORPALETTENODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "graph/parameter/colorpaletteparameter.h"
#include "graph/parameter/colorparameter.h"

namespace photon {

class FadeColorPaletteNode: public keira::Node
{
public:
    FadeColorPaletteNode();


    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_blendParam;
    keira::OptionParameter *m_modeParam;
    ColorPaletteParameter *m_paletteParam;
    ColorParameter *m_colorParam;
};

} // namespace photon

#endif // FADECOLORPALETTENODE_H
