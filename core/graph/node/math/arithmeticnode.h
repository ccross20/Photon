#ifndef PHOTON_ARITHMETICNODE_H
#define PHOTON_ARITHMETICNODE_H
#include "model/node.h"
#include "photon-global.h"

namespace photon {

class  PHOTONCORE_EXPORT ArithmeticNode : public keira::Node
{
public:
    const static QByteArray InputA;
    const static QByteArray InputB;
    const static QByteArray Mode;
    const static QByteArray Output;

    ArithmeticNode();
    ~ArithmeticNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // PHOTON_ARITHMETICNODE_H
