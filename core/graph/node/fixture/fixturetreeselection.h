#ifndef FIXTURETREESELECTION_H
#define FIXTURETREESELECTION_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureTreeSelection : public keira::Node
{
public:
    const static QByteArray Fixture;

    FixtureTreeSelection();
    ~FixtureTreeSelection();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // FIXTURETREESELECTION_H
