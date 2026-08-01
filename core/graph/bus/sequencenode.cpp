#include "sequencenode.h"
#include "graph/parameter/dmxmatrixparameter.h"
#include "model/parameter/buttonparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "sequence/sequence.h"
#include "sequence/sequencecollection.h"
#include "photoncore.h"

namespace photon {

const QByteArray SequenceNode::InputDMX = "dmxInput";
const QByteArray SequenceNode::OutputDMX = "dmxOutput";
const QByteArray SequenceNode::SequenceParam = "sequence";

class SequenceNode::Impl
{
public:
    DMXMatrixParameter *dmxInParam = nullptr;
    DMXMatrixParameter *dmxOutParam = nullptr;
    keira::ButtonParameter *editParam = nullptr;
    keira::StringOptionParameter *sequenceParam = nullptr;
};

keira::NodeInformation SequenceNode::info()
{
    keira::NodeInformation toReturn([](){return new SequenceNode;});
    toReturn.name = "Sequence Node";
    toReturn.nodeId = "photon.bus.sequence";

    return toReturn;
}

SequenceNode::SequenceNode() : keira::Node("photon.bus.sequence"),m_impl(new Impl)
{
    setName("Sequence");
}

SequenceNode::~SequenceNode()
{
    delete m_impl;
}

void SequenceNode::createParameters()
{
    m_impl->dmxInParam = new DMXMatrixParameter(InputDMX,"DMX Input", DMXMatrix());
    addParameter(m_impl->dmxInParam);

    // Dropdown of the project's sequences. The lambda re-lists them whenever the
    // editor builds the combo, so it stays in sync with the sequence collection
    // (same pattern as FixtureGroupNode). Stored/matched by name - sequences have
    // no stable id beyond that today, so renaming a sequence will detach this node
    // from it.
    m_impl->sequenceParam = new keira::StringOptionParameter(SequenceParam, "Sequence", {}, 0);
    m_impl->sequenceParam->setOptionLambda([]() {
        QVector<std::pair<QString, QString>> options;
        if(SequenceCollection *sequences = photonApp->sequences())
        {
            for(Sequence *seq : sequences->sequences())
                options.append({seq->name(), seq->name()});
        }
        return options;
    });
    addParameter(m_impl->sequenceParam);

    m_impl->editParam = new keira::ButtonParameter("Edit","Edit");
    m_impl->editParam->setLayoutOptions(keira::Parameter::LayoutNoLabel);
    addParameter(m_impl->editParam);

    m_impl->dmxOutParam = new DMXMatrixParameter(OutputDMX,"DMX Output", DMXMatrix(), keira::AllowMultipleOutput);
    addParameter(m_impl->dmxOutParam);
}

void SequenceNode::evaluate(keira::EvaluationContext *) const
{
    DMXMatrix matrix = m_impl->dmxInParam->value().value<DMXMatrix>();

    // Resolve and process the chosen sequence directly - a plain model object, not
    // a QWidget - so this is safe to call from the eval thread. (Previously this
    // reached into whichever SequencePanel happened to be open in the editor and
    // called into its QWidget-based preview path from here, which is not
    // thread-safe and could corrupt memory in ways unrelated to this node.)
    const QString name = m_impl->sequenceParam->value().toString();
    if(SequenceCollection *sequences = photonApp->sequences())
    {
        for(Sequence *seq : sequences->sequences())
        {
            if(seq->name() == name)
            {
                ProcessContext context{matrix};
                context.project = photonApp->project();
                // Preview at wherever the sequence's own editor playhead currently is
                // (Sequence::previewTime(), kept up to date by SequenceWidget) so this
                // node reflects live scrubbing/playback instead of always evaluating
                // at time 0.
                context.globalTime = seq->previewTime();
                seq->processChannels(context, 0);
                break;
            }
        }
    }

    m_impl->dmxOutParam->setValue(matrix);
}

void SequenceNode::buttonClicked(const keira::Parameter *)
{
    const QString name = m_impl->sequenceParam->value().toString();
    if(SequenceCollection *sequences = photonApp->sequences())
    {
        for(Sequence *seq : sequences->sequences())
        {
            if(seq->name() == name)
            {
                sequences->editSequence(seq);
                break;
            }
        }
    }
}

} // namespace photon
