#ifndef PHOTON_DMXWRITERNODE_H
#define PHOTON_DMXWRITERNODE_H
#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT DMXWriterNode : public keira::Node
{
public:


    const static QByteArray InputDMX;

    DMXWriterNode();
    ~DMXWriterNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DMXWRITERNODE_H
