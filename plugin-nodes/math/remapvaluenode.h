#ifndef REMAPVALUENODE_H
#define REMAPVALUENODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

class RemapValueNode : public keira::Node
{
public:
    RemapValueNode();
    ~RemapValueNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;

    keira::DecimalParameter *m_inputParam;
    keira::DecimalParameter *m_minInParam;
    keira::DecimalParameter *m_maxInParam;
    keira::DecimalParameter *m_minOutParam;
    keira::DecimalParameter *m_maxOutParam;
    keira::DecimalParameter *m_outputParam;
    keira::OptionParameter *m_easeTypeParam;
    keira::OptionParameter *m_boundsParam;
};

} // namespace photon

#endif // REMAPVALUENODE_H
