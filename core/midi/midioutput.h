#ifndef PHOTON_MIDIOUTPUT_H
#define PHOTON_MIDIOUTPUT_H

#include <QByteArray>
#include <QStringList>
#include "photon-global.h"

namespace photon {

// Sends MIDI out of Photon (macOS: CoreMIDI; a no-op elsewhere). Messages go
// to a destination picked by name - any MIDI destination the system offers,
// e.g. a Network MIDI session reaching QuickShow on another machine - or to
// Photon's own virtual source (VirtualDeviceName), which local apps see as an
// input called "Photon".
//
// Thread-safe: graph nodes send from the evaluation thread.
class PHOTONCORE_EXPORT MidiOutput
{
public:
    static const QString VirtualDeviceName;

    static MidiOutput *instance();

    // VirtualDeviceName first, then the system's MIDI destinations.
    QStringList deviceNames() const;

    // Channels are 1-16; data bytes are clamped to 0-127.
    void noteOn(const QString &device, int channel, int note, int velocity);
    void noteOff(const QString &device, int channel, int note);
    void controlChange(const QString &device, int channel, int controller, int value);

private:
    MidiOutput();
    ~MidiOutput();
    void send(const QString &device, const QByteArray &message);

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MIDIOUTPUT_H
