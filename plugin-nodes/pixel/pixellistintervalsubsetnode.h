#ifndef PIXELLISTINTERVALSUBSETNODE_H
#define PIXELLISTINTERVALSUBSETNODE_H
#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/booleanparameter.h"
#include "photon-global.h"

namespace photon {

// Picks pixels in a repeating select/skip pattern: Select Count pixels, then
// skip Skip Count, and so on, starting at Offset (taken modulo the pixel
// count, negative counts back from the end). Without Loop the walk stops at
// the end of the list; with Loop it wraps back to the start and carries on
// until it reaches Offset again, so every pixel gets its turn in the pattern
// whatever the offset. Output is in walk order, starting at the Offset pixel.
class PixelListIntervalSubsetNode : public keira::Node
{
public:
    const static QByteArray PixelsInParam;
    const static QByteArray OffsetParam;
    const static QByteArray SelectCountParam;
    const static QByteArray SkipCountParam;
    const static QByteArray LoopParam;
    const static QByteArray PixelsOutParam;

    PixelListIntervalSubsetNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    PixelListParameter *m_inParam;
    keira::IntegerParameter *m_offsetParam;
    keira::IntegerParameter *m_selectParam;
    keira::IntegerParameter *m_skipParam;
    keira::BooleanParameter *m_loopParam;
    PixelListParameter *m_outParam;
};

} // namespace photon

#endif // PIXELLISTINTERVALSUBSETNODE_H
