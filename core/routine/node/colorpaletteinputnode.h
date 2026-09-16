#ifndef PHOTON_COLORPALETTEINPUTNODE_H
#define PHOTON_COLORPALETTEINPUTNODE_H

#include "model/graphinputnode.h"
#include "photon-global.h"

namespace photon {

// Exposes a ColorPalette (a plain list of colors) as an input on the
// enclosing subgraph node (Fixture/Canvas/Pixel Graph), the same way Number/
// Color/Point/Boolean/Integer/String Input do. No Default field like those:
// unlike them, a palette has no matching ChannelInfo::ChannelType, so it
// never becomes a sequence-animatable clip channel with its own seeded
// keyframe - it's only ever wired in from a parent graph. Not usable inside
// a Routine for the same reason.
class PHOTONCORE_EXPORT ColorPaletteInputNode : public keira::GraphInputNode
{
public:
    const static QByteArray Value;
    const static QByteArray Name;
    const static QByteArray Description;

    ColorPaletteInputNode();
    ~ColorPaletteInputNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    QString portName() const override;

    static keira::NodeInformation info();
    void setValue(const QByteArray &t_id, const QVariant &t_value) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORPALETTEINPUTNODE_H
