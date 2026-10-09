#include "midicuenode.h"
#include "midi/midioutput.h"

namespace photon {

keira::NodeInformation MidiCueNode::info()
{
    keira::NodeInformation toReturn([](){return new MidiCueNode;});
    toReturn.name = "MIDI Cue";
    toReturn.nodeId = "photon.midi.cue";
    toReturn.categories = {"MIDI"};
    return toReturn;
}

MidiCueNode::MidiCueNode() : keira::Node("photon.midi.cue")
{
    setName("MIDI Cue");
}

void MidiCueNode::createParameters()
{
    m_deviceParam = new keira::StringOptionParameter("device", "Device", {}, 0);
    m_deviceParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        for(const QString &name : MidiOutput::instance()->deviceNames())
            options.append({name, name});
        return options;
    });
    m_deviceParam->setValue(MidiOutput::VirtualDeviceName);
    addParameter(m_deviceParam);

    // QuickShow routes each MIDI channel to a page.
    m_pageParam = new keira::IntegerParameter("page", "Page (MIDI Channel)", 1);
    m_pageParam->setMinimum(1);
    m_pageParam->setMaximum(16);
    addParameter(m_pageParam);

    // Match QuickShow's "First key" (MIDI Settings): the note for cue 1.
    m_firstNoteParam = new keira::IntegerParameter("firstNote", "First Note", 36);
    m_firstNoteParam->setMinimum(0);
    m_firstNoteParam->setMaximum(127);
    addParameter(m_firstNoteParam);

    m_cueParam = new keira::IntegerParameter("cue", "Cue", 0);
    m_cueParam->setMinimum(0);
    m_cueParam->setMaximum(128);
    addParameter(m_cueParam);
}

void MidiCueNode::evaluate(keira::EvaluationContext *) const
{
    const QString device = m_deviceParam->value().toString();
    const int page = m_pageParam->value().toInt();
    const int cue = m_cueParam->value().toInt();
    const int note = cue > 0 ? m_firstNoteParam->value().toInt() + cue - 1 : -1;

    if(device == m_lastDevice && page == m_lastPage && note == m_lastNote)
        return;
    m_lastDevice = device;
    m_lastPage = page;
    m_lastNote = note;

    if(note < 0 || note > 127)
        return;

    // A tap, like pressing the key: QuickShow triggers on the note-on.
    MidiOutput::instance()->noteOn(device, page, note, 127);
    MidiOutput::instance()->noteOff(device, page, note);
}

} // namespace photon
