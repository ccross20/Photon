#ifndef PHOTON_CANVASREADER_H
#define PHOTON_CANVASREADER_H


#include "model/node.h"
#include "graph/parameter/canvasparameter.h"

namespace photon {

class CanvasReader : public keira::Node
{
public:
    CanvasReader();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    CanvasParameter *m_canvasParam;
};

} // namespace photon

#endif // PHOTON_CANVASREADER_H
