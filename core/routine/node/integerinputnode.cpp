#include "integerinputnode.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/stringparameter.h"
#include "routine/routineevaluationcontext.h"
#include "routine/routine.h"

namespace photon {

const QByteArray IntegerInputNode::Value = "value";
const QByteArray IntegerInputNode::Name = "name";
const QByteArray IntegerInputNode::Description = "description";
const QByteArray IntegerInputNode::DefaultValue = "default";

class IntegerInputNode::Impl
{
public:
    keira::IntegerParameter *valueParam;
    keira::IntegerParameter *defaultValueParam;
    keira::StringParameter *nameParam;
    keira::StringParameter *descriptionParam;
    uint index = 0;
};


keira::NodeInformation IntegerInputNode::info()
{
    keira::NodeInformation toReturn([](){return new IntegerInputNode;});
    toReturn.name = "Integer Input";
    toReturn.nodeId = "photon.routine.integer-input";
    toReturn.categories = {"Input"};
    toReturn.graphs = QByteArrayList{"routine","fixture","canvas","pixel"};
    toReturn.inputParameterType = keira::IntegerParameter::ParameterId;

    return toReturn;
}

IntegerInputNode::IntegerInputNode() : keira::GraphInputNode("photon.routine.integer-input"),m_impl(new Impl)
{
    setName("Integer Input");
    setValuePortId(Value);
}

IntegerInputNode::~IntegerInputNode()
{
    delete m_impl;
}

void IntegerInputNode::markDirty(int t_dirty)
{
    Node::markDirty(t_dirty);

    if(m_impl->defaultValueParam->isDirty() || m_impl->nameParam->isDirty())
    {
        if(auto *routine = dynamic_cast<Routine*>(graph()))
            routine->updateChannel(m_impl->index, channelInfo());
        notifyInterfaceChanged();
    }
}

QString IntegerInputNode::portName() const
{
    return m_impl->nameParam->value().toString();
}

ChannelInfo IntegerInputNode::channelInfo() const
{
    ChannelInfo info;
    info.type = ChannelInfo::ChannelTypeInteger;
    info.name = m_impl->nameParam->value().toString();
    info.description = m_impl->descriptionParam->value().toString();
    info.defaultValue = m_impl->defaultValueParam->value();
    info.uniqueId = uniqueId();

    return info;
}

void IntegerInputNode::setChannelIndex(uint t_index)
{
    m_impl->index = t_index;
}

uint IntegerInputNode::channelIndex() const
{
    return m_impl->index;
}

void IntegerInputNode::createParameters()
{
    m_impl->valueParam = new keira::IntegerParameter(Value,"Value", 0, keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);
    m_impl->nameParam = new keira::StringParameter(Name,"Name", "Integer", 0);
    addParameter(m_impl->nameParam);
    m_impl->descriptionParam = new keira::StringParameter(Description,"Description", "", 0);
    addParameter(m_impl->descriptionParam);
    m_impl->defaultValueParam = new keira::IntegerParameter(DefaultValue,"Default", 0, keira::AllowSingleInput);
    addParameter(m_impl->defaultValueParam);
}

void IntegerInputNode::setValue(const QByteArray &t_id, const QVariant &t_value)
{
    keira::Node::setValue(t_id, t_value);

    if(t_id == IntegerInputNode::Name || t_id == IntegerInputNode::DefaultValue)
    {
        if(auto *routine = dynamic_cast<Routine*>(graph()))
            routine->updateChannel(channelIndex(), channelInfo());
        notifyInterfaceChanged();
    }

}

void IntegerInputNode::evaluate(keira::EvaluationContext *) const
{
    // Subgraph path relies on the enclosing SubGraphNode's applyInputs() having
    // set the value port already; see NumberInputNode::evaluate.
}

void IntegerInputNode::readFromJson(const QJsonObject &t_object, keira::NodeLibrary *t_library)
{
    Node::readFromJson(t_object, t_library);
    m_impl->index = t_object.value("index").toInt();
}

void IntegerInputNode::writeToJson(QJsonObject &t_object) const
{
    Node::writeToJson(t_object);
    t_object.insert("index", (int)m_impl->index);
}

} // namespace photon
