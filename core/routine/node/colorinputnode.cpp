#include "colorinputnode.h"
#include "model/parameter/stringparameter.h"
#include "graph/parameter/colorparameter.h"
#include "routine/routineevaluationcontext.h"
#include "routine/routine.h"

namespace photon {

const QByteArray ColorInputNode::Value = "value";
const QByteArray ColorInputNode::Name = "name";
const QByteArray ColorInputNode::Description = "description";
const QByteArray ColorInputNode::DefaultValue = "default";

class ColorInputNode::Impl
{
public:
    ColorParameter *valueParam;
    ColorParameter *defaultValueParam;
    keira::StringParameter *nameParam;
    keira::StringParameter *descriptionParam;
    uint index = 0;
};


keira::NodeInformation ColorInputNode::info()
{
    keira::NodeInformation toReturn([](){return new ColorInputNode;});
    toReturn.name = "Color Input";
    toReturn.nodeId = "photon.routine.color-input";

    return toReturn;
}

ColorInputNode::ColorInputNode() : keira::Node("photon.routine.color-input"),m_impl(new Impl)
{
    setName("Color Input");
}

ColorInputNode::~ColorInputNode()
{
    delete m_impl;
}

void ColorInputNode::markDirty()
{
    Node::markDirty();

    if(m_impl->defaultValueParam->isDirty() || m_impl->nameParam->isDirty())
        static_cast<Routine*>(graph())->updateChannel(m_impl->index, channelInfo());
}

ChannelInfo ColorInputNode::channelInfo() const
{
    ChannelInfo info;
    info.type = ChannelInfo::ChannelTypeColor;
    info.name = m_impl->nameParam->value().toString();
    info.description = m_impl->descriptionParam->value().toString();
    info.defaultValue = m_impl->defaultValueParam->value();
    info.uniqueId = uniqueId();

    return info;
}

void ColorInputNode::setChannelIndex(uint t_index)
{
    m_impl->index = t_index;
}

uint ColorInputNode::channelIndex() const
{
    return m_impl->index;
}

void ColorInputNode::createParameters()
{
    m_impl->valueParam = new ColorParameter(Value,"Value", QColor(Qt::white), keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);
    m_impl->nameParam = new keira::StringParameter(Name,"Name", "Color", 0);
    addParameter(m_impl->nameParam);
    m_impl->descriptionParam = new keira::StringParameter(Description,"Description", "", 0);
    addParameter(m_impl->descriptionParam);
    m_impl->defaultValueParam = new ColorParameter(DefaultValue,"Value", QColor(Qt::white), keira::AllowSingleInput);
    addParameter(m_impl->defaultValueParam);
}

void ColorInputNode::setValue(const QByteArray &t_id, const QVariant &t_value)
{
    keira::Node::setValue(t_id, t_value);

    if(t_id == ColorInputNode::Name || t_id == ColorInputNode::DefaultValue)
        static_cast<Routine*>(graph())->updateChannel(channelIndex(), channelInfo());

}

void ColorInputNode::evaluate(keira::EvaluationContext *t_context) const
{
    //RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    //m_impl->valueParam->setValue(context->channelValues.value(uniqueId(),m_impl->defaultValueParam->value()));

    //qDebug() << context->strength << context->relativeTime;
}

void ColorInputNode::readFromJson(const QJsonObject &t_object, keira::NodeLibrary *t_library)
{
    Node::readFromJson(t_object, t_library);
    m_impl->index = t_object.value("index").toInt();
}

void ColorInputNode::writeToJson(QJsonObject &t_object) const
{
    Node::writeToJson(t_object);
    t_object.insert("index", (int)m_impl->index);
}

} // namespace photon

