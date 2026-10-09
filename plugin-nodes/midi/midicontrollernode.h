#ifndef MIDICONTROLLERNODE_H
#define MIDICONTROLLERNODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "photon-global.h"

namespace photon {

// Sends a value as a MIDI controller (CC) message: Value 0..1 maps to 0..127,
// sent whenever that changes. For live controls an app learns by MIDI (e.g.
// QuickShow's MIDI Live Control: size, position, brightness, ...).
class MidiControllerNode : public keira::Node
{
public:
    MidiControllerNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::StringOptionParameter *m_deviceParam;
    keira::IntegerParameter *m_channelParam;
    keira::IntegerParameter *m_controllerParam;
    keira::DecimalParameter *m_valueParam;

    // What was last sent, so a message goes out only on change.
    mutable QString m_lastDevice;
    mutable int m_lastChannel = -1;
    mutable int m_lastController = -1;
    mutable int m_lastValue = -1;
};

} // namespace photon

#endif // MIDICONTROLLERNODE_H
