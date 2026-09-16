#include "booleaninputnode.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/stringparameter.h"
#include "routine/routineevaluationcontext.h"
#include "routine/routine.h"

namespace photon {

const QByteArray BooleanInputNode::Value = "value";
const QByteArray BooleanInputNode::Name = "name";
const QByteArray BooleanInputNode::Description = "description";
const QByteArray BooleanInputNode::DefaultValue = "default";

class BooleanInputNode::Impl
{
public:
    keira::BooleanParameter *valueParam;
    keira::BooleanParameter *defaultValueParam;
    keira::StringParameter *nameParam;
    keira::StringParameter *descriptionParam;
    uint index = 0;
};


keira::NodeInformation BooleanInputNode::info()
{
    keira::NodeInformation toReturn([](){return new BooleanInputNode;});
    toReturn.name = "Boolean Input";
    toReturn.nodeId = "photon.routine.boolean-input";
    toReturn.categories = {"Input"};
    toReturn.graphs = QByteArrayList{"routine","fixture","canvas","pixel"};
    toReturn.inputParameterType = keira::BooleanParameter::ParameterId;

    return toReturn;
}

BooleanInputNode::BooleanInputNode() : keira::GraphInputNode("photon.routine.boolean-input"),m_impl(new Impl)
{
    setName("Boolean Input");
    setValuePortId(Value);
}

BooleanInputNode::~BooleanInputNode()
{
    delete m_impl;
}

void BooleanInputNode::markDirty(int t_dirty)
{
    Node::markDirty(t_dirty);

    if(m_impl->defaultValueParam->isDirty() || m_impl->nameParam->isDirty())
    {
        if(auto *routine = dynamic_cast<Routine*>(graph()))
            routine->updateChannel(m_impl->index, channelInfo());
        notifyInterfaceChanged();
    }
}

QString BooleanInputNode::portName() const
{
    return m_impl->nameParam->value().toString();
}

ChannelInfo BooleanInputNode::channelInfo() const
{
    ChannelInfo info;
    info.type = ChannelInfo::ChannelTypeBool;
    info.name = m_impl->nameParam->value().toString();
    info.description = m_impl->descriptionParam->value().toString();
    info.defaultValue = m_impl->defaultValueParam->value();
    info.uniqueId = uniqueId();

    return info;
}

void BooleanInputNode::setChannelIndex(uint t_index)
{
    m_impl->index = t_index;
}

uint BooleanInputNode::channelIndex() const
{
    return m_impl->index;
}

void BooleanInputNode::createParameters()
{
    m_impl->valueParam = new keira::BooleanParameter(Value,"Value", false, keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);
    m_impl->nameParam = new keira::StringParameter(Name,"Name", "Boolean", 0);
    addParameter(m_impl->nameParam);
    m_impl->descriptionParam = new keira::StringParameter(Description,"Description", "", 0);
    addParameter(m_impl->descriptionParam);
    m_impl->defaultValueParam = new keira::BooleanParameter(DefaultValue,"Default", false, keira::AllowSingleInput);
    addParameter(m_impl->defaultValueParam);
}

void BooleanInputNode::setValue(const QByteArray &t_id, const QVariant &t_value)
{
    keira::Node::setValue(t_id, t_value);

    if(t_id == BooleanInputNode::Name || t_id == BooleanInputNode::DefaultValue)
    {
        if(auto *routine = dynamic_cast<Routine*>(graph()))
            routine->updateChannel(channelIndex(), channelInfo());
        notifyInterfaceChanged();
    }

}

void BooleanInputNode::evaluate(keira::EvaluationContext *) const
{
    // Subgraph path relies on the enclosing SubGraphNode's applyInputs() having
    // set the value port already; see NumberInputNode::evaluate.
}

void BooleanInputNode::readFromJson(const QJsonObject &t_object, keira::NodeLibrary *t_library)
{
    Node::readFromJson(t_object, t_library);
    m_impl->index = t_object.value("index").toInt();
}

void BooleanInputNode::writeToJson(QJsonObject &t_object) const
{
    Node::writeToJson(t_object);
    t_object.insert("index", (int)m_impl->index);
}

} // namespace photon
