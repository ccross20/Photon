#ifndef BUTTONGIZMONODE_H
#define BUTTONGIZMONODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/stringoptionparameter.h"
#include "model/parameter/booleanparameter.h"

namespace photon {

class PHOTONCORE_EXPORT ButtonGizmoNode : public keira::Node
{
public:
    const static QByteArray GizmoId;
    const static QByteArray Value;

    ButtonGizmoNode();


    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

protected:
    void addedToGraph(keira::Graph*) override;

private:
    keira::StringOptionParameter *m_optionParam;
    keira::BooleanParameter *m_valueParam;
};

} // namespace photon

#endif // BUTTONGIZMONODE_H
