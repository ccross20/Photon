#ifndef PHOTON_SAVEDPALETTENODE_H
#define PHOTON_SAVEDPALETTENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs a named, project-level saved colour palette (managed in the
// project panel's "Color Palettes" section), resolved live each evaluation -
// same pattern as FixtureGroupNode, just for the Color Palettes library.
class PHOTONCORE_EXPORT SavedPaletteNode : public keira::Node
{
public:
    const static QByteArray PaletteParam;
    const static QByteArray ResultParam;

    SavedPaletteNode();
    ~SavedPaletteNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SAVEDPALETTENODE_H
