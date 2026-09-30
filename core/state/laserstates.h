#ifndef PHOTON_LASERSTATES_H
#define PHOTON_LASERSTATES_H

#include "statecapability.h"

namespace photon {

// State capabilities for laser fixtures, each driving a group of the
// fixture's LaserCapability channels. Offered by LaserStateNode.

class PHOTONCORE_EXPORT LaserContentState : public StateCapability
{
public:
    LaserContentState();
    void evaluate(const StateEvaluationContext &) const override;
};

class PHOTONCORE_EXPORT LaserSizeState : public StateCapability
{
public:
    LaserSizeState();
    void evaluate(const StateEvaluationContext &) const override;
};

class PHOTONCORE_EXPORT LaserPositionState : public StateCapability
{
public:
    LaserPositionState();
    void evaluate(const StateEvaluationContext &) const override;
};

class PHOTONCORE_EXPORT LaserRotationState : public StateCapability
{
public:
    LaserRotationState();
    void evaluate(const StateEvaluationContext &) const override;
};

class PHOTONCORE_EXPORT LaserColorState : public StateCapability
{
public:
    LaserColorState();
    void evaluate(const StateEvaluationContext &) const override;
};

class PHOTONCORE_EXPORT LaserScanRateState : public StateCapability
{
public:
    LaserScanRateState();
    void evaluate(const StateEvaluationContext &) const override;
};

class PHOTONCORE_EXPORT LaserVisiblePointsState : public StateCapability
{
public:
    LaserVisiblePointsState();
    void evaluate(const StateEvaluationContext &) const override;
};

class PHOTONCORE_EXPORT LaserStrobeState : public StateCapability
{
public:
    LaserStrobeState();
    void evaluate(const StateEvaluationContext &) const override;
};

} // namespace photon

#endif // PHOTON_LASERSTATES_H
