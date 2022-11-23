#ifndef PHOTON_TRIGONOMETRYNODE_H
#define PHOTON_TRIGONOMETRYNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT TrigonometryNode : public keira::Node
{
public:
    const static QByteArray InputA;
    const static QByteArray Output;

    TrigonometryNode();
    ~TrigonometryNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TRIGONOMETRYNODE_H
