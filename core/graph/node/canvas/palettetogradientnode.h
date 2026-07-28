#ifndef PHOTON_PALETTETOGRADIENTNODE_H
#define PHOTON_PALETTETOGRADIENTNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class ColorPaletteParameter;
class GradientParameter;

// Builds a Gradient from a colour palette, spacing the palette's colours evenly
// across the [0,1] range. A helper for feeding CanvasGradientNode.
class PHOTONCORE_EXPORT PaletteToGradientNode : public keira::Node
{
public:
    const static QByteArray Palette;
    const static QByteArray Output;

    PaletteToGradientNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    ColorPaletteParameter *m_palette = nullptr;
    GradientParameter *m_output = nullptr;
};

} // namespace photon

#endif // PHOTON_PALETTETOGRADIENTNODE_H
