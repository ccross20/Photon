#ifndef MIDICUENODE_H
#define MIDICUENODE_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "photon-global.h"

namespace photon {

// Triggers a cue in a MIDI-driven cue grid such as Pangolin QuickShow's.
// Whenever the cue (or page) changes it sends a short tap - note on, then off
// - for note First Note + Cue - 1 on the page's MIDI channel, matching
// QuickShow's layout: its "First key" triggers the first cue, higher notes
// later ones, and each MIDI channel can be routed to a page. Cue 0 sends
// nothing.
class MidiCueNode : public keira::Node
{
public:
    MidiCueNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::StringOptionParameter *m_deviceParam;
    keira::IntegerParameter *m_pageParam;
    keira::IntegerParameter *m_firstNoteParam;
    keira::IntegerParameter *m_cueParam;

    // What was last sent, so a tap goes out only when the cue changes.
    mutable QString m_lastDevice;
    mutable int m_lastPage = -1;
    mutable int m_lastNote = -1;
};

} // namespace photon

#endif // MIDICUENODE_H
