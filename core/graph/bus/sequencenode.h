#ifndef PHOTON_SEQUENCENODE_H
#define PHOTON_SEQUENCENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SequenceNode : public keira::Node
{
public:
    const static QByteArray InputDMX;
    const static QByteArray OutputDMX;

    SequenceNode();
    ~SequenceNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    void buttonClicked(const keira::Parameter *) override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SEQUENCENODE_H
