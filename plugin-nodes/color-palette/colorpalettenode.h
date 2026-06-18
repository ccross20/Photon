#ifndef COLORPALETTENODE_H
#define COLORPALETTENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {


class ColorPaletteNode : public keira::Node
{
public:
    const static QByteArray PaletteParam;
    ColorPaletteNode();


    void createParameters() override;


    static keira::NodeInformation info();
};

} // namespace photon

#endif // COLORPALETTENODE_H
