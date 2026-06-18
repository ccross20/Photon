#ifndef COLORFROMCOLORPALETTE_H
#define COLORFROMCOLORPALETTE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/integerparameter.h"
#include "graph/parameter/colorpaletteparameter.h"
#include "graph/parameter/colorparameter.h"

namespace photon {

class ColorFromColorPalette : public keira::Node
{
public:
    ColorFromColorPalette();


    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();

private:
    keira::IntegerParameter *m_indexParam;
    ColorPaletteParameter *m_paletteParam;
    ColorParameter *m_colorParam;
};

} // namespace photon

#endif // COLORFROMCOLORPALETTE_H
