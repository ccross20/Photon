#ifndef DMXREADERNODE_H
#define DMXREADERNODE_H
#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT DMXReaderNode : public keira::Node
{
public:


    const static QByteArray OutputDMX;

    DMXReaderNode();
    ~DMXReaderNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // DMXREADERNODE_H
