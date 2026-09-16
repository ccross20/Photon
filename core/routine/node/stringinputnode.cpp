#include "stringinputnode.h"
#include "model/parameter/stringparameter.h"
#include "routine/routineevaluationcontext.h"
#include "routine/routine.h"

namespace photon {

const QByteArray StringInputNode::Value = "value";
const QByteArray StringInputNode::Name = "name";
const QByteArray StringInputNode::Description = "description";
const QByteArray StringInputNode::DefaultValue = "default";

class StringInputNode::Impl
{
public:
    keira::StringParameter *valueParam;
    keira::StringParameter *defaultValueParam;
    keira::StringParameter *nameParam;
    keira::StringParameter *descriptionParam;
    uint index = 0;
};


keira::NodeInformation StringInputNode::info()
{
    keira::NodeInformation toReturn([](){return new StringInputNode;});
    toReturn.name = "String Input";
    toReturn.nodeId = "photon.routine.string-input";
    toReturn.categories = {"Input"};
    toReturn.graphs = QByteArrayList{"routine","fixture","canvas","pixel"};
    toReturn.inputParameterType = keira::StringParameter::ParameterId;

    return toReturn;
}

StringInputNode::StringInputNode() : keira::GraphInputNode("photon.routine.string-input"),m_impl(new Impl)
{
    setName("String Input");
    setValuePortId(Value);
}

StringInputNode::~StringInputNode()
{
    delete m_impl;
}

void StringInputNode::markDirty(int t_dirty)
{
    Node::markDirty(t_dirty);

    if(m_impl->defaultValueParam->isDirty() || m_impl->nameParam->isDirty())
    {
        if(auto *routine = dynamic_cast<Routine*>(graph()))
            routine->updateChannel(m_impl->index, channelInfo());
        notifyInterfaceChanged();
    }
}

QString StringInputNode::portName() const
{
    return m_impl->nameParam->value().toString();
}

ChannelInfo StringInputNode::channelInfo() const
{
    ChannelInfo info;
    info.type = ChannelInfo::ChannelTypeString;
    info.name = m_impl->nameParam->value().toString();
    info.description = m_impl->descriptionParam->value().toString();
    info.defaultValue = m_impl->defaultValueParam->value();
    info.uniqueId = uniqueId();

    return info;
}

void StringInputNode::setChannelIndex(uint t_index)
{
    m_impl->index = t_index;
}

uint StringInputNode::channelIndex() const
{
    return m_impl->index;
}

void StringInputNode::createParameters()
{
    m_impl->valueParam = new keira::StringParameter(Value,"Value", "", keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);
    m_impl->nameParam = new keira::StringParameter(Name,"Name", "String", 0);
    addParameter(m_impl->nameParam);
    m_impl->descriptionParam = new keira::StringParameter(Description,"Description", "", 0);
    addParameter(m_impl->descriptionParam);
    m_impl->defaultValueParam = new keira::StringParameter(DefaultValue,"Default", "", keira::AllowSingleInput);
    addParameter(m_impl->defaultValueParam);
}

void StringInputNode::setValue(const QByteArray &t_id, const QVariant &t_value)
{
    keira::Node::setValue(t_id, t_value);

    if(t_id == StringInputNode::Name || t_id == StringInputNode::DefaultValue)
    {
        if(auto *routine = dynamic_cast<Routine*>(graph()))
            routine->updateChannel(channelIndex(), channelInfo());
        notifyInterfaceChanged();
    }

}

void StringInputNode::evaluate(keira::EvaluationContext *) const
{
    // Subgraph path relies on the enclosing SubGraphNode's applyInputs() having
    // set the value port already; see NumberInputNode::evaluate.
}

void StringInputNode::readFromJson(const QJsonObject &t_object, keira::NodeLibrary *t_library)
{
    Node::readFromJson(t_object, t_library);
    m_impl->index = t_object.value("index").toInt();
}

void StringInputNode::writeToJson(QJsonObject &t_object) const
{
    Node::writeToJson(t_object);
    t_object.insert("index", (int)m_impl->index);
}

} // namespace photon
