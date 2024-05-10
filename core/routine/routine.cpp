#include "routine.h"
#include <math.h>
#include "data/dmxmatrix.h"
#include "model/node.h"
#include "routineevaluationcontext.h"
#include "node/numberinputnode.h"
#include "node/baseroutinenode.h"
#include "node/colorinputnode.h"
#include "node/pointinputnode.h"
#include "opengl/openglframebuffer.h"

namespace photon {

class Routine::Impl
{
public:
    QString name;
    QVector<ChannelInfo> channels;
    QSet<BaseRoutineNode*> toInitialize;
};

Routine::Routine(const QString &t_name, QObject *parent)
    : keira::Graph{parent},m_impl(new Impl)
{
    m_impl->name = t_name;

    /*
    ChannelInfo info;
    info.name = "Strength";
    info.description = "How much of the effect to blend.";
    info.type = ChannelInfo::ChannelTypeNumber;
    info.defaultValue = 1.0;
    m_impl->channels.append(info);
*/
}

Routine::~Routine()
{
    delete m_impl;
}

void Routine::initializeContext(QOpenGLContext *t_context, Canvas *t_canvas)
{
    for(auto node : nodes())
    {
        auto baseNode = dynamic_cast<BaseRoutineNode*>(node);
        if(baseNode)
        {
            baseNode->initializeContext(t_context, t_canvas);
        }
    }
}

void Routine::canvasResized(QOpenGLContext *t_context, Canvas *t_canvas)
{
    for(auto node : nodes())
    {
        auto baseNode = dynamic_cast<BaseRoutineNode*>(node);
        if(baseNode)
        {
            baseNode->canvasResized(t_context, t_canvas);
        }
    }
}

QString Routine::name() const
{
    return m_impl->name;
}

void Routine::setName(const QString &t_value)
{
    m_impl->name = t_value;
}

const QVector<ChannelInfo> &Routine::channelInfo() const
{
    return m_impl->channels;
}

void Routine::evaluate(keira::EvaluationContext *t_context) const
{
    auto routineContext = static_cast<RoutineEvaluationContext*>(t_context);

    if(routineContext)
    {
        for(auto it = m_impl->toInitialize.cbegin(); it != m_impl->toInitialize.cend(); ++it)
        {
            (*it)->initializeContext(routineContext->openglContext, routineContext->canvas);
        }
        m_impl->toInitialize.clear();

        Graph::evaluate(routineContext);
    }
    else
    {
        DMXMatrix matrix;
        RoutineEvaluationContext localContext(matrix);

        Graph::evaluate(&localContext);
    }

}

void Routine::nodeAdded(keira::Node *t_node)
{
    NumberInputNode *input = dynamic_cast<NumberInputNode *>(t_node);
    if(input)
    {
        //info.name = input->findParameter(NumberInputNode::Name)->
        addChannel(input->channelInfo());
        input->setChannelIndex(m_impl->channels.length()-1);
    }
    ColorInputNode *colorInput = dynamic_cast<ColorInputNode *>(t_node);
    if(colorInput)
    {
        //info.name = input->findParameter(NumberInputNode::Name)->
        addChannel(colorInput->channelInfo());
        colorInput->setChannelIndex(m_impl->channels.length()-1);
    }
    PointInputNode *pointInput = dynamic_cast<PointInputNode *>(t_node);
    if(pointInput)
    {
        //info.name = input->findParameter(NumberInputNode::Name)->
        addChannel(pointInput->channelInfo());
        pointInput->setChannelIndex(m_impl->channels.length()-1);
    }

    auto baseNode = dynamic_cast<BaseRoutineNode*>(t_node);
    if(baseNode)
        m_impl->toInitialize << baseNode;
}

void Routine::nodeRemoved(keira::Node *t_node)
{
    NumberInputNode *input = dynamic_cast<NumberInputNode *>(t_node);
    if(input)
    {
        removeChannel(input->channelIndex());
    }
    ColorInputNode *colorInput = dynamic_cast<ColorInputNode *>(t_node);
    if(colorInput)
    {
        removeChannel(colorInput->channelIndex());
    }
    PointInputNode *pointInput = dynamic_cast<PointInputNode *>(t_node);
    if(pointInput)
    {
        removeChannel(pointInput->channelIndex());
    }

    auto baseNode = dynamic_cast<BaseRoutineNode*>(t_node);
    if(baseNode)
        m_impl->toInitialize.remove(baseNode);
}

void Routine::addChannel(const ChannelInfo &info)
{
    m_impl->channels.append(info);
    emit channelAdded(m_impl->channels.length()-1);
}

void Routine::updateChannel(int index, const ChannelInfo &info)
{
    if(index < m_impl->channels.length())
    {
        m_impl->channels[index] = info;
        emit channelUpdated(index);
    }
    else
        qWarning() << "Index " << index << "out of bounds";
}

void Routine::removeChannel(int index)
{
    if(index < m_impl->channels.length())
    {
        m_impl->channels.removeAt(index);
        emit channelRemoved(index);
    }
    else
        qWarning() << "Index " << index << "out of bounds";

}

int Routine::channelCount() const
{
    return m_impl->channels.length();
}

ChannelInfo Routine::channelInfoAtIndex(int index)
{
    return m_impl->channels.at(index);
}

void Routine::readFromJson(const QJsonObject &t_json, keira::NodeLibrary *t_library)
{
    Graph::readFromJson(t_json, t_library);

    m_impl->name = t_json["name"].toString();

    for(auto node : nodes())
    {
        NumberInputNode *input = dynamic_cast<NumberInputNode *>(node);
        if(input)
        {
            m_impl->channels.append(input->channelInfo());
        }
        ColorInputNode *colorInput = dynamic_cast<ColorInputNode *>(node);
        if(colorInput)
        {
            m_impl->channels.append(colorInput->channelInfo());
        }
        PointInputNode *pointInput = dynamic_cast<PointInputNode *>(node);
        if(pointInput)
        {
            m_impl->channels.append(pointInput->channelInfo());
        }
    }
}

void Routine::writeToJson(QJsonObject &t_json) const
{
    Graph::writeToJson(t_json);

    t_json.insert("name",m_impl->name);
}

} // namespace photon
