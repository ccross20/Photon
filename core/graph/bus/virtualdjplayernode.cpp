#include "virtualdjplayernode.h"
#include "graph/bus/busgraph.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/decimalparameter.h"
#include "sequence/sequence.h"
#include "virtualdj/virtualdjplayback.h"
#include "photoncore.h"

namespace photon {

const QByteArray VirtualDJPlayerNode::InputDMX = "dmxInput";
const QByteArray VirtualDJPlayerNode::OutputDMX = "dmxOutput";
const QByteArray VirtualDJPlayerNode::EnabledParam = "enabled";
const QByteArray VirtualDJPlayerNode::OffsetParam = "offset";

class VirtualDJPlayerNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam = nullptr;
    DMXMatrixParameter *dmxOutParam = nullptr;
    keira::BooleanParameter *enabledParam = nullptr;
    keira::DecimalParameter *offsetParam = nullptr;

    mutable Sequence *lastSequence = nullptr;
    mutable double lastTime = 0.0;
};

keira::NodeInformation VirtualDJPlayerNode::info()
{
    keira::NodeInformation toReturn([](){return new VirtualDJPlayerNode;});
    toReturn.name = "VirtualDJ Player";
    toReturn.nodeId = "photon.bus.virtualdj-player";
    toReturn.graphs = QByteArrayList{BusGraph::BusGraphId};
    return toReturn;
}

VirtualDJPlayerNode::VirtualDJPlayerNode() : keira::Node("photon.bus.virtualdj-player"), m_impl(new Impl)
{
    setName("VirtualDJ Player");
    // The song position moves every frame without any parameter changing.
    setIsAlwaysDirty(true);
}

VirtualDJPlayerNode::~VirtualDJPlayerNode()
{
    delete m_impl;
}

void VirtualDJPlayerNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX, "DMX Input", DMXMatrix());
    addParameter(m_impl->dmxInParam);

    m_impl->enabledParam = new keira::BooleanParameter(EnabledParam, "Enabled", true);
    addParameter(m_impl->enabledParam);

    // Positive runs the lights ahead of the audio, to cover output latency.
    m_impl->offsetParam = new keira::DecimalParameter(OffsetParam, "Offset (ms)", 0.0);
    m_impl->offsetParam->setSoftRange(-500.0, 500.0);
    m_impl->offsetParam->setPrecision(0);
    addParameter(m_impl->offsetParam);

    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX, "DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    addParameter(m_impl->dmxOutParam);
}

void VirtualDJPlayerNode::evaluate(keira::EvaluationContext *) const
{
    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();

    VirtualDJPlayback *playback = photonApp->djPlayback();
    if(m_impl->enabledParam->value().toBool() && playback)
    {
        const double offset = m_impl->offsetParam->value().toDouble() / 1000.0;
        playback->process([this, &matrix, offset](Sequence *sequence, double songTime) {
            const double time = songTime + offset;
            // A different sequence (track change) starts with no history.
            if(sequence != m_impl->lastSequence)
            {
                m_impl->lastSequence = sequence;
                m_impl->lastTime = time;
            }

            ProcessContext context{matrix};
            context.project = photonApp->project();
            context.globalTime = time;
            sequence->processChannels(context, m_impl->lastTime);
            m_impl->lastTime = time;
        });
    }

    m_impl->dmxOutParam->setValue(matrix);
}

} // namespace photon
