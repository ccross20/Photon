#include <QLabel>
#include "parameter_p.h"
#include "../node.h"

namespace keira {

Parameter::Impl::Impl(Parameter *t_param):facade(t_param){}

void Parameter::Impl::connectOutput(Parameter *t_input)
{
    if(outputParams.contains(t_input))
        return;
    outputParams.append(t_input);
}

void Parameter::Impl::connectInput(Parameter *t_output)
{
    if(inputParams.contains(t_output))
        return;
    inputParams.append(t_output);
    facade->setValue(t_output->value());
}

void Parameter::Impl::disconnectOutput(Parameter *t_input)
{
    outputParams.removeOne(t_input);
}

void Parameter::Impl::disconnectInput(Parameter *t_output)
{
    inputParams.removeOne(t_output);
    facade->setValue(defaultValue);
}

Parameter::Parameter():m_impl(new Impl(this))
{

}

Parameter::Parameter(const QByteArray &t_typeId, const QByteArray &t_id, const QString &t_name, const QVariant &t_default, int t_connectionOptions):m_impl(new Impl(this))
{
    m_impl->typeId = t_typeId;
    m_impl->id = t_id;
    m_impl->name = t_name;
    m_impl->value = t_default;
    m_impl->defaultValue = t_default;
    m_impl->connectionOptions = t_connectionOptions;

}

Parameter::~Parameter()
{
    delete m_impl;
}

QByteArray Parameter::id() const
{
    return m_impl->id;
}

int Parameter::layoutOptions() const
{
    return m_impl->layoutOptions;
}

void Parameter::setLayoutOptions(int t_value)
{
    m_impl->layoutOptions = t_value;
}

bool Parameter::isReadOnly() const
{
    return !(m_impl->connectionOptions & AllowSingleInput || m_impl->connectionOptions & AllowMultipleInput);
}

QByteArray Parameter::typeId() const
{
    return m_impl->typeId;
}

QString Parameter::name() const
{
    return m_impl->name;
}

QString Parameter::description() const
{
    return m_impl->description;
}

int Parameter::rowHeight() const
{
    return 40;
}

bool Parameter::hasInput() const
{
    return !m_impl->inputParams.isEmpty();
}

bool Parameter::hasOutput() const
{
    return !m_impl->outputParams.isEmpty();
}

bool Parameter::allowMultipleInput() const
{
    return m_impl->connectionOptions & AllowMultipleInput;
}

bool Parameter::allowMultipleOutput() const
{
    return m_impl->connectionOptions & AllowMultipleOutput;
}

bool Parameter::allowInput() const
{
    return m_impl->connectionOptions & AllowSingleInput || m_impl->connectionOptions & AllowMultipleInput;
}

bool Parameter::allowOutput() const
{
    return m_impl->connectionOptions & AllowSingleOutput || m_impl->connectionOptions & AllowMultipleOutput;
}

Node *Parameter::node() const
{
    return m_impl->node;
}

bool Parameter::canConvert(const QByteArray &) const
{
    return false;
}

QVariant Parameter::convertTo(const QByteArray &) const
{
    return QVariant();
}

const QVector<Parameter*> &Parameter::outputParameters() const
{
    return m_impl->outputParams;
}

const QVector<Parameter*> &Parameter::inputParameters() const
{
    return m_impl->inputParams;
}

Parameter *Parameter::inputParameter() const
{
    if(!m_impl->inputParams.isEmpty())
        return m_impl->inputParams.front();
    return nullptr;
}


const QVariant &Parameter::value() const
{
    return m_impl->value;
}

void Parameter::setValue(const QVariant &t_value)
{
    if(m_impl->testValueForChange && t_value == m_impl->value)
        return;
    m_impl->value = t_value;
    markDirty();

    //qDebug() << "Param set value" << id() << m_impl->value;
    for(Parameter *param : m_impl->outputParams)
        param->setValue(t_value);
}

bool Parameter::testValueForChange() const
{
    return m_impl->testValueForChange;
}

void Parameter::setTestValueForChange(bool t_value)
{
    m_impl->testValueForChange = t_value;
}

bool Parameter::isDirty() const
{
    return m_impl->isDirty;
}


void Parameter::validate(const QVariant &)
{

}

QWidget *Parameter::createWidget(NodeItem *) const
{
    QLabel *label = new QLabel(name());
    label->setMaximumHeight(24);
    return label;
}

void Parameter::updateWidget(QWidget *) const
{

}

QVariant Parameter::updateValue(QWidget *) const
{
    return QVariant();
}

void Parameter::markDirty()
{
    //qDebug() << "Param dirty" << m_impl->isDirty;
    if(m_impl->isDirty)
        return;

    m_impl->isDirty = true;
    if(m_impl->node)
        m_impl->node->markDirty();
}

void Parameter::readFromJson(const QJsonObject &t_json)
{
    m_impl->id = t_json.value("id").toString().toLatin1();
    m_impl->typeId = t_json.value("typeId").toString().toLatin1();
    m_impl->name = t_json.value("name").toString();
    m_impl->description = t_json.value("description").toString();
    m_impl->value = t_json.value("value");
    m_impl->testValueForChange = t_json.value("testValueForChange").toBool(true);
    m_impl->defaultValue = t_json.value("defaultValue");
    m_impl->layoutOptions = t_json.value("layoutOptions").toInt();
    m_impl->connectionOptions = t_json.value("connectionOptions").toInt();
}

void Parameter::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("id", QString(m_impl->id));
    t_json.insert("typeId", QString(m_impl->typeId));
    t_json.insert("name", m_impl->name);
    t_json.insert("description", m_impl->description);
    t_json.insert("value", m_impl->value.toJsonValue());
    t_json.insert("defaultValue", m_impl->defaultValue.toJsonValue());
    t_json.insert("layoutOptions", m_impl->layoutOptions);
    t_json.insert("connectionOptions", m_impl->connectionOptions);
    t_json.insert("testValueForChange", m_impl->testValueForChange);
}


} // namespace keira
