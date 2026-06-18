#ifndef SLIDERGIZMONODE_H
#define SLIDERGIZMONODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/stringoptionparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class PHOTONCORE_EXPORT SliderGizmoNode : public keira::Node
{
public:
    const static QByteArray GizmoId;
    const static QByteArray Value;

    SliderGizmoNode();


    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

protected:
    void addedToGraph(keira::Graph*) override;

private:
    keira::StringOptionParameter *m_optionParam;
    keira::DecimalParameter *m_valueParam;
};

} // namespace photon

#endif // SLIDERGIZMONODE_H
