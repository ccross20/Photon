#include "pointinputnode.h"

#include "model/parameter/decimalparameter.h"
#include "model/parameter/stringparameter.h"
#include "graph/parameter/point2dparameter.h"
#include "routine/routineevaluationcontext.h"
#include "routine/routine.h"

namespace photon {

const QByteArray PointInputNode::Value = "value";
const QByteArray PointInputNode::Name = "name";
const QByteArray PointInputNode::Description = "description";
const QByteArray PointInputNode::DefaultValue = "default";

class PointInputNode::Impl
{
public:
    Point2DParameter *valueParam;
    Point2DParameter *defaultValueParam;
    keira::StringParameter *nameParam;
    keira::StringParameter *descriptionParam;
    uint index;
};


keira::NodeInformation PointInputNode::info()
{
    keira::NodeInformation toReturn([](){return new PointInputNode;});
    toReturn.name = "Point Input";
    toReturn.nodeId = "photon.routine.point-input";

    return toReturn;
}

PointInputNode::PointInputNode() : keira::Node("photon.routine.point-input"),m_impl(new Impl)
{
    setName("Point Input");
    setWidth(300);
}

PointInputNode::~PointInputNode()
{
    delete m_impl;
}

void PointInputNode::markDirty()
{
    Node::markDirty();

    if(m_impl->defaultValueParam->isDirty() || m_impl->nameParam->isDirty())
        static_cast<Routine*>(graph())->updateChannel(m_impl->index, channelInfo());
}

void PointInputNode::setValue(const QByteArray &t_id, const QVariant &t_value)
{
    keira::Node::setValue(t_id, t_value);

    if(t_id == PointInputNode::Name || t_id == PointInputNode::DefaultValue)
        static_cast<Routine*>(graph())->updateChannel(channelIndex(), channelInfo());

}

ChannelInfo PointInputNode::channelInfo() const
{
    ChannelInfo info;
    info.type = ChannelInfo::ChannelTypePoint;
    info.name = m_impl->nameParam->value().toString();
    info.description = m_impl->descriptionParam->value().toString();
    info.defaultValue = m_impl->defaultValueParam->value();
    info.uniqueId = uniqueId();

    return info;
}

void PointInputNode::setChannelIndex(uint t_index)
{
    m_impl->index = t_index;
}

uint PointInputNode::channelIndex() const
{
    return m_impl->index;
}

void PointInputNode::createParameters()
{
    m_impl->valueParam = new Point2DParameter(Value,"Value", QPointF(), keira::AllowMultipleOutput);
    addParameter(m_impl->valueParam);
    m_impl->nameParam = new keira::StringParameter(Name,"Name", "Number", 0);
    addParameter(m_impl->nameParam);
    m_impl->descriptionParam = new keira::StringParameter(Description,"Description", "", 0);
    addParameter(m_impl->descriptionParam);
    m_impl->defaultValueParam = new Point2DParameter(DefaultValue,"Default", QPointF(), keira::AllowSingleInput);
    addParameter(m_impl->defaultValueParam);
}

void PointInputNode::evaluate(keira::EvaluationContext *t_context) const
{
    //RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);

    //m_impl->valueParam->setValue(context->channelValues.value(uniqueId(),m_impl->defaultValueParam->value()));
}

void PointInputNode::readFromJson(const QJsonObject &t_object, keira::NodeLibrary *t_library)
{
    Node::readFromJson(t_object, t_library);
    m_impl->index = t_object.value("index").toInt();
}

void PointInputNode::writeToJson(QJsonObject &t_object) const
{
    Node::writeToJson(t_object);
    t_object.insert("index", (int)m_impl->index);
}

} // namespace photon
