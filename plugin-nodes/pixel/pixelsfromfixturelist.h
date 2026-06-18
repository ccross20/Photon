#ifndef PIXELSFROMFIXTURELIST_H
#define PIXELSFROMFIXTURELIST_H
#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PixelsFromFixtureList : public keira::Node
{
public:

    PixelsFromFixtureList();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;


    static keira::NodeInformation info();

private:
    FixtureListParameter *m_inParam;
    PixelListParameter *m_pixelsOut;
};

} // namespace photon

#endif // PIXELSFROMFIXTURELIST_H
