#include <algorithm>
#include <QMutex>
#include "midioutput.h"

#ifdef Q_OS_MACOS
#include <CoreMIDI/CoreMIDI.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace photon {

const QString MidiOutput::VirtualDeviceName = QStringLiteral("Photon (virtual)");

namespace {

int clamp7(int t_value)
{
    return std::clamp(t_value, 0, 127);
}

int statusByte(int t_kind, int t_channel)
{
    return t_kind | (std::clamp(t_channel, 1, 16) - 1);
}

QByteArray message(int t_status, int t_data1, int t_data2)
{
    QByteArray bytes(3, Qt::Uninitialized);
    bytes[0] = char(t_status);
    bytes[1] = char(t_data1);
    bytes[2] = char(t_data2);
    return bytes;
}

} // namespace

class MidiOutput::Impl
{
public:
    QMutex mutex;
#ifdef Q_OS_MACOS
    MIDIClientRef client = 0;
    MIDIPortRef port = 0;
    MIDIEndpointRef virtualSource = 0;

    static QString endpointName(MIDIEndpointRef t_endpoint)
    {
        CFStringRef name = nullptr;
        if(MIDIObjectGetStringProperty(t_endpoint, kMIDIPropertyDisplayName, &name) != noErr || !name)
            return QString();
        const QString result = QString::fromCFString(name);
        CFRelease(name);
        return result;
    }

    MIDIEndpointRef findDestination(const QString &t_name) const
    {
        const ItemCount count = MIDIGetNumberOfDestinations();
        for(ItemCount i = 0; i < count; ++i)
        {
            const MIDIEndpointRef endpoint = MIDIGetDestination(i);
            if(endpointName(endpoint) == t_name)
                return endpoint;
        }
        return 0;
    }
#endif
};

MidiOutput *MidiOutput::instance()
{
    static MidiOutput output;
    return &output;
}

MidiOutput::MidiOutput() : m_impl(new Impl)
{
#ifdef Q_OS_MACOS
    if(MIDIClientCreate(CFSTR("Photon"), nullptr, nullptr, &m_impl->client) == noErr)
    {
        MIDIOutputPortCreate(m_impl->client, CFSTR("Photon Out"), &m_impl->port);
        MIDISourceCreate(m_impl->client, CFSTR("Photon"), &m_impl->virtualSource);
    }
#endif
}

MidiOutput::~MidiOutput()
{
#ifdef Q_OS_MACOS
    if(m_impl->client)
        MIDIClientDispose(m_impl->client);
#endif
    delete m_impl;
}

QStringList MidiOutput::deviceNames() const
{
    QStringList names{VirtualDeviceName};
#ifdef Q_OS_MACOS
    const ItemCount count = MIDIGetNumberOfDestinations();
    for(ItemCount i = 0; i < count; ++i)
    {
        const QString name = Impl::endpointName(MIDIGetDestination(i));
        if(!name.isEmpty() && !names.contains(name))
            names.append(name);
    }
#endif
    return names;
}

void MidiOutput::noteOn(const QString &t_device, int t_channel, int t_note, int t_velocity)
{
    send(t_device, message(statusByte(0x90, t_channel), clamp7(t_note), clamp7(t_velocity)));
}

void MidiOutput::noteOff(const QString &t_device, int t_channel, int t_note)
{
    send(t_device, message(statusByte(0x80, t_channel), clamp7(t_note), 0));
}

void MidiOutput::controlChange(const QString &t_device, int t_channel, int t_controller, int t_value)
{
    send(t_device, message(statusByte(0xB0, t_channel), clamp7(t_controller), clamp7(t_value)));
}

void MidiOutput::send(const QString &t_device, const QByteArray &t_message)
{
#ifdef Q_OS_MACOS
    QMutexLocker lock(&m_impl->mutex);
    if(!m_impl->client)
        return;

    Byte buffer[256];
    auto *list = reinterpret_cast<MIDIPacketList *>(buffer);
    MIDIPacket *packet = MIDIPacketListInit(list);
    packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0, ByteCount(t_message.size()),
                               reinterpret_cast<const Byte *>(t_message.constData()));
    if(!packet)
        return;

    if(t_device.isEmpty() || t_device == VirtualDeviceName)
    {
        if(m_impl->virtualSource)
            MIDIReceived(m_impl->virtualSource, list);
        return;
    }

    // Looked up by name each time - only a handful of destinations, and it
    // picks up a session that was connected after Photon started.
    if(const MIDIEndpointRef destination = m_impl->findDestination(t_device))
        MIDISend(m_impl->port, destination, list);
#else
    Q_UNUSED(t_device)
    Q_UNUSED(t_message)
#endif
}

} // namespace photon
