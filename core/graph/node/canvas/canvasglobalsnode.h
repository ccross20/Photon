#ifndef PHOTON_CANVASGLOBALSNODE_H
#define PHOTON_CANVASGLOBALSNODE_H

#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

// Auto-created "Globals" node inside a canvas subgraph. Exposes the per-frame
// canvas constants (resolution + time) as outputs that other canvas nodes read.
// The CanvasSubGraphNode pushes fresh values onto it each evaluation.
class PHOTONCORE_EXPORT CanvasGlobalsNode : public keira::Node
{
public:
    const static QByteArray WidthParam;
    const static QByteArray HeightParam;
    const static QByteArray TimeParam;
    const static QByteArray GlobalTimeParam;

    CanvasGlobalsNode();

    void createParameters() override;
    static keira::NodeInformation info();

private:
    keira::IntegerParameter *m_widthParam;
    keira::IntegerParameter *m_heightParam;
    keira::DecimalParameter *m_timeParam;
    keira::DecimalParameter *m_globalTimeParam;
};

} // namespace photon

#endif // PHOTON_CANVASGLOBALSNODE_H
