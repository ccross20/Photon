#ifndef PHOTON_DMXGENERATEMATRIXNODE_H
#define PHOTON_DMXGENERATEMATRIXNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT DMXGenerateMatrixNode : public keira::Node
{
public:


    const static QByteArray OutputDMX;

    DMXGenerateMatrixNode();
    ~DMXGenerateMatrixNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon


#endif // PHOTON_DMXGENERATEMATRIXNODE_H
