#ifndef PIXELLISTRANDOMIZENODE_H
#define PIXELLISTRANDOMIZENODE_H
#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "photon-global.h"

namespace photon {

// Shuffles a pixel list into a random order. The order depends only on the
// seed (and the list), so the same seed always gives the same shuffle.
class PixelListRandomizeNode : public keira::Node
{
public:
    const static QByteArray PixelsInParam;
    const static QByteArray SeedParam;
    const static QByteArray PixelsOutParam;

    PixelListRandomizeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    PixelListParameter *m_inParam;
    keira::IntegerParameter *m_seedParam;
    PixelListParameter *m_outParam;
};

} // namespace photon

#endif // PIXELLISTRANDOMIZENODE_H
