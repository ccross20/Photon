#include "routine.h"
#include <math.h>
#include "data/dmxmatrix.h"
#include "model/node.h"
#include "routineevaluationcontext.h"
#include "node/numberinputnode.h"

namespace photon {

class Routine::Impl
{
public:
    QString name;
    QVector<ChannelInfo> channels;
};

Routine::Routine(const QString &t_name, QObject *parent)
    : keira::Graph{parent},m_impl(new Impl)
{
    m_impl->name = t_name;

    ChannelInfo info;
    info.name = "Strength";
    info.description = "How much of the effect to blend.";
    info.type = ChannelInfo::ChannelTypeNumber;
    info.defaultValue = 1.0;
    m_impl->channels.append(info);
}

Routine::~Routine()
{
    delete m_impl;
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

void Routine::evaluate(keira::EvaluationContext *context) const
{
    if(dynamic_cast<RoutineEvaluationContext*>(context))
    {
        Graph::evaluateAll(context);
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
}

void Routine::nodeRemoved(keira::Node *t_node)
{
    NumberInputNode *input = dynamic_cast<NumberInputNode *>(t_node);
    if(input)
    {

        //info.name = input->findParameter(NumberInputNode::Name)->
        removeChannel(input->channelIndex());
    }
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
}

void Routine::writeToJson(QJsonObject &t_json) const
{
    Graph::writeToJson(t_json);

    t_json.insert("name",m_impl->name);
}

} // namespace photon
