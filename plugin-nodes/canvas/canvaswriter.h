#ifndef PHOTON_CANVASWRITER_H
#define PHOTON_CANVASWRITER_H

#include "model/node.h"
#include "graph/parameter/canvasparameter.h"

namespace photon {

class CanvasWriter : public keira::Node
{
public:
    CanvasWriter();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    CanvasParameter *m_canvasParam;
};

} // namespace photon

#endif // PHOTON_CANVASWRITER_H
