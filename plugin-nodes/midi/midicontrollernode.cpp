#include <algorithm>
#include <cmath>
#include "midicontrollernode.h"
#include "midi/midioutput.h"

namespace photon {

keira::NodeInformation MidiControllerNode::info()
{
    keira::NodeInformation toReturn([](){return new MidiControllerNode;});
    toReturn.name = "MIDI Controller";
    toReturn.nodeId = "photon.midi.controller";
    toReturn.categories = {"MIDI"};
    return toReturn;
}

MidiControllerNode::MidiControllerNode() : keira::Node("photon.midi.controller")
{
    setName("MIDI Controller");
}

void MidiControllerNode::createParameters()
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

    m_channelParam = new keira::IntegerParameter("channel", "Channel", 1);
    m_channelParam->setMinimum(1);
    m_channelParam->setMaximum(16);
    addParameter(m_channelParam);

    m_controllerParam = new keira::IntegerParameter("controller", "Controller", 1);
    m_controllerParam->setMinimum(0);
    m_controllerParam->setMaximum(127);
    addParameter(m_controllerParam);

    m_valueParam = new keira::DecimalParameter("value", "Value", 0.0);
    m_valueParam->setMinimum(0.0);
    m_valueParam->setMaximum(1.0);
    addParameter(m_valueParam);
}

void MidiControllerNode::evaluate(keira::EvaluationContext *) const
{
    const QString device = m_deviceParam->value().toString();
    const int channel = m_channelParam->value().toInt();
    const int controller = m_controllerParam->value().toInt();
    const int value = int(std::lround(std::clamp(m_valueParam->value().toDouble(), 0.0, 1.0) * 127.0));

    if(device == m_lastDevice && channel == m_lastChannel && controller == m_lastController && value == m_lastValue)
        return;
    m_lastDevice = device;
    m_lastChannel = channel;
    m_lastController = controller;
    m_lastValue = value;

    MidiOutput::instance()->controlChange(device, channel, controller, value);
}

} // namespace photon
