#ifndef PHOTON_BEATREDUCERNODE_H
#define PHOTON_BEATREDUCERNODE_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "photon-global.h"

namespace photon {

// Thins out a beat count so downstream triggers fire on every Nth beat instead
// of every beat. Skip is how many beats to drop between hits (Skip 1 = every
// other beat, Skip 3 = every 4th); Offset shifts which beats land on the grid.
// The output holds the last beat that hit, so an "on value change" trigger sees
// one change per (Skip + 1) beats.
class BeatReducerNode : public keira::Node
{
public:
    BeatReducerNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::IntegerParameter *m_beatInParam;
    keira::IntegerParameter *m_skipParam;
    keira::IntegerParameter *m_offsetParam;
    keira::IntegerParameter *m_beatOutParam;
};

} // namespace photon

#endif // PHOTON_BEATREDUCERNODE_H
