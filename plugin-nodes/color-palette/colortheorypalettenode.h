#ifndef COLORTHEORYPALETTENODE_H
#define COLORTHEORYPALETTENODE_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "graph/parameter/colorparameter.h"
#include "graph/parameter/colorpaletteparameter.h"
#include "photon-global.h"

namespace photon {

// Turns one base color into a whole palette, using classic color-theory
// relationships (complementary, triadic, ...) plus a couple of generative
// ones (Golden Angle, Random Jitter) for when you just want N distinct,
// good-looking colors without picking a specific harmony.
class ColorTheoryPaletteNode : public keira::Node
{
public:
    enum Mode
    {
        ModeComplementary,
        ModeSplitComplementary,
        ModeAnalogous,
        ModeTriadic,
        ModeTetradicSquare,
        ModeTetradicRectangle,
        ModeMonochromatic,
        ModeShades,
        ModeTints,
        ModeTones,
        ModeCompound,
        ModeGoldenAngle,
        ModeRandomJitter,
    };

    ColorTheoryPaletteNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    ColorParameter *m_colorParam = nullptr;
    keira::OptionParameter *m_modeParam = nullptr;
    keira::IntegerParameter *m_countParam = nullptr;
    keira::DecimalParameter *m_spreadParam = nullptr;
    keira::DecimalParameter *m_saturationParam = nullptr;
    keira::DecimalParameter *m_brightnessParam = nullptr;
    keira::IntegerParameter *m_seedParam = nullptr;
    ColorPaletteParameter *m_paletteParam = nullptr;
};

} // namespace photon

#endif // COLORTHEORYPALETTENODE_H
