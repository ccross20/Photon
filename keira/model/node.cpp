#include <QUuid>
#include "node_p.h"
#include "graph.h"
#include "parameter/parameter_p.h"
#include "parameter/decimalparameter.h"
#include "parameter/buttonparameter.h"
#include "library/nodelibrary.h"

namespace keira {

Node::Node(const QByteArray &t_id)
    : m_impl(new Impl)
{
    m_impl->id = t_id;
    m_impl->uniqueId = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
}
Node::~Node()
{
    for(Parameter *param : m_impl->parameters)
        delete param;
    delete m_impl;
}

void Node::setWidth(double t_value)
{
    m_impl->width = t_value;
}

double Node::width() const
{
    return m_impl->width;
}

void Node::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QString Node::name() const
{
    return m_impl->name;
}

void Node::setId(const QByteArray &t_id)
{
    m_impl->id = t_id;
}

QByteArray Node::id() const
{
    return m_impl->id;
}

void Node::createParameters()
{

}

Graph *Node::graph() const
{
    return m_impl->graph;
}

QPointF Node::position() const
{
    return m_impl->position;
}

void Node::setPosition(const QPointF &t_position)
{
    if(m_impl->position == t_position)
        return;
    m_impl->position = t_position;
    //emit positionUpdated(this);
}

void Node::addParameter(Parameter *t_param)
{
    if(std::any_of(m_impl->parameters.cbegin(), m_impl->parameters.cend(),[t_param](Parameter *testParam){return t_param->name() == testParam->name();}))
        Q_ASSERT("Parameter names must be unique.");
    m_impl->parameters.append(t_param);
    t_param->m_impl->node = this;
}

void Node::removeParameter(Parameter *t_param)
{
    m_impl->parameters.removeOne(t_param);
    t_param->m_impl->node = nullptr;
}

void Node::updateParameter(Parameter *t_param)
{
    if(std::any_of(m_impl->parameters.cbegin(), m_impl->parameters.cend(),[t_param](Parameter *testParam){
                   return t_param->name() == testParam->name() && t_param != testParam;
            }))
        Q_ASSERT("Parameter names must be unique.");
}

const QVector<Parameter*> &Node::parameters() const
{
    return m_impl->parameters;
}

Parameter *Node::findParameter(const QByteArray &t_paramId) const
{
    for(auto it = m_impl->parameters.cbegin(); it != m_impl->parameters.cend(); ++it)
    {
        if((*it)->id() == t_paramId)
            return *it;
    }
    return nullptr;
}


void Node::setValue(const QByteArray &t_id, const QVariant &t_value)
{
    Parameter *param = findParameter(t_id);
    if(param)
        param->setValue(t_value);
    else
        qDebug() << "Could not find parameter:" << t_id;
}

const QVariant &Node::value(const QByteArray &t_id) const
{
    Parameter *param = findParameter(t_id);
    if(param)
        return param->value();
    else
        qDebug() << "Could not find parameter:" << t_id;
    return m_impl->emptyValue;
}

QByteArray Node::uniqueId() const
{
    return m_impl->uniqueId;
}

void Node::setIsAlwaysDirty(bool t_value)
{
    m_impl->isAlwaysDirty = t_value;
}

bool Node::isAlwaysDirty() const
{
    return m_impl->isAlwaysDirty;
}

void Node::evaluate(EvaluationContext *) const
{

}

void Node::buttonClicked(const Parameter *)
{

}

void Node::markDirty()
{
    if(m_impl->isDirty)
        return;
    m_impl->isDirty = true;
    for(auto it = m_impl->dependents.cbegin(); it != m_impl->dependents.cend(); ++it)
        (*it)->markDirty();

   graph()->markDirty();
}

void Node::markClean()
{
    m_impl->isDirty = false;
    for(Parameter *param : m_impl->parameters)
        param->m_impl->isDirty = false;
}

bool Node::isDirty() const
{
    return m_impl->isAlwaysDirty || m_impl->isDirty;
}

void Node::inputParameterConnected(Parameter* t_param)
{

}

void Node::outputParameterConnected(Parameter* t_param)
{
    m_impl->dependents.insert(t_param->node());
}

void Node::inputParameterDisconnected(Parameter*)
{

}

void Node::outputParameterDisconnected(Parameter* t_param)
{
    Node *nodeToCheck = t_param->node();
    for(Parameter *param : m_impl->parameters)
    {
        if(param->node() == nodeToCheck)
            return;
    }
    m_impl->dependents.remove(nodeToCheck);
}

void Node::readFromJson(const QJsonObject &t_json, NodeLibrary *library)
{
    m_impl->id = t_json.value("id").toString().toLatin1();
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();

    QJsonObject positionObject = t_json.value("position").toObject();
    m_impl->position = QPointF{positionObject.value("x").toDouble(), positionObject.value("y").toDouble()};
    m_impl->name = t_json.value("name").toString();

    auto parameterArray = t_json.value("parameters").toArray();
    for(const auto &paramJson : parameterArray)
    {
        const auto &paramObj = paramJson.toObject();
        Parameter *param = findParameter(paramObj.value("id").toString().toLatin1());

        if(!param)
        {
            QString typeId = paramObj.value("typeId").toString();
            param = library->createParameter(typeId.toLatin1());
            m_impl->parameters.append(param);
        }

        if(param)
        {
            param->readFromJson(paramObj);
        }
    }
}

void Node::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("id", QString(m_impl->id));
    t_json.insert("uniqueId", QString(m_impl->uniqueId));

    QJsonObject positionObject;
    positionObject.insert("x", m_impl->position.x());
    positionObject.insert("y", m_impl->position.y());
    t_json.insert("position", positionObject);
    t_json.insert("name", m_impl->name);

    QJsonArray paramArray;

    for(auto param : m_impl->parameters)
    {
        QJsonObject paramObj;
        param->writeToJson(paramObj);
        paramArray.append(paramObj);
    }
    t_json.insert("parameters", paramArray);
}


} // namespace keira
