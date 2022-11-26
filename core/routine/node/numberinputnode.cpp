#include "numberinputnode.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/stringparameter.h"
#include "routine/routineevaluationcontext.h"
#include "routine/routine.h"

namespace photon {

const QByteArray NumberInputNode::Value = "value";
const QByteArray NumberInputNode::Name = "name";
const QByteArray NumberInputNode::Description = "description";
const QByteArray NumberInputNode::DefaultValue = "default";

class NumberInputNode::Impl
{
public:
    keira::DecimalParameter *valueParam;
    keira::DecimalParameter *defaultValueParam;
    keira::StringParameter *nameParam;
    keira::StringParameter *descriptionParam;
    uint index;
};


keira::NodeInformation NumberInputNode::info()
{
    keira::NodeInformation toReturn([](){return new NumberInputNode;});
    toReturn.name = "Number Input";
    toReturn.nodeId = "photon.routine.number-input";

    return toReturn;
}

NumberInputNode::NumberInputNode() : keira::Node("photon.routine.number-input"),m_impl(new Impl)
{
    setName("Number Input");
}

NumberInputNode::~NumberInputNode()
{
    delete m_impl;
}

void NumberInputNode::markDirty()
{
    Node::markDirty();

    if(m_impl->defaultValueParam->isDirty() || m_impl->nameParam->isDirty())
        static_cast<Routine*>(graph())->updateChannel(m_impl->index, channelInfo());
}

ChannelInfo NumberInputNode::channelInfo() const
{
    ChannelInfo info;
    info.type = ChannelInfo::ChannelTypeNumber;
    info.name = m_impl->nameParam->value().toString();
    info.description = m_impl->descriptionParam->value().toString();
    info.defaultValue = m_impl->defaultValueParam->value();

    return info;
}

void NumberInputNode::setChannelIndex(uint t_index)
{
    m_impl->index = t_index;
}

uint NumberInputNode::channelIndex() const
{
    return m_impl->index;
}

void NumberInputNode::createParameters()
{
    m_impl->valueParam = new keira::DecimalParameter(Value,"Value", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);
    m_impl->nameParam = new keira::StringParameter(Name,"Name", "Number", 0);
    addParameter(m_impl->nameParam);
    m_impl->descriptionParam = new keira::StringParameter(Description,"Description", "", 0);
    addParameter(m_impl->descriptionParam);
    m_impl->defaultValueParam = new keira::DecimalParameter(DefaultValue,"Default", 0, 0);
    addParameter(m_impl->defaultValueParam);
}

void NumberInputNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    if(m_impl->index < context->channelValues.length())
    m_impl->valueParam->setValue(context->channelValues[m_impl->index]);

    //qDebug() << context->strength << context->relativeTime;
}

void NumberInputNode::readFromJson(const QJsonObject &t_object, keira::NodeLibrary *t_library)
{
    Node::readFromJson(t_object, t_library);
    m_impl->index = t_object.value("index").toInt();
}

void NumberInputNode::writeToJson(QJsonObject &t_object) const
{
    Node::writeToJson(t_object);
    t_object.insert("index", (int)m_impl->index);
}

} // namespace photon
