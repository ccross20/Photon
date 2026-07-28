#include <QLabel>
#include "integerparameter.h"
#include "decimalparameter.h"
#include "view/nodeeditor.h"
#include "view/numberscrubfield.h"

namespace keira {

const QByteArray IntegerParameter::ParameterId = "integer";

bool IntegerParameter::acceptsConnectionFrom(const Parameter *source) const
{
    return Parameter::acceptsConnectionFrom(source)
        || source->typeId() == DecimalParameter::ParameterId;
}

class IntegerParameter::Impl
{
public:
    int minimum = std::numeric_limits<int>::lowest();
    int maximum = std::numeric_limits<int>::max();
};

IntegerParameter::IntegerParameter() : Parameter(),m_impl(new Impl)
{

}

IntegerParameter::IntegerParameter(const QByteArray &t_id, const QString &t_name, int t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{

}

IntegerParameter::~IntegerParameter()
{
    delete m_impl;
}

void IntegerParameter::setMinimum(int t_min)
{
    m_impl->minimum = t_min;
}

void IntegerParameter::setMaximum(int t_max)
{
    m_impl->maximum = t_max;
}

QWidget *IntegerParameter::createWidget(NodeEditor *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }
    NumberScrubField *field = new NumberScrubField();
    field->setMaximumHeight(30);
    field->setMinimumWidth(50);
    field->setIsInteger(true);
    field->setRange(m_impl->minimum, m_impl->maximum);
    field->setValue(value().toInt());
    field->setReadOnly(isReadOnly());

    const IntegerParameter *param = this;
    NumberScrubField::connect(field, &NumberScrubField::editingFinished, field,[item, field, param](){item->widgetUpdated(field, param);});
    NumberScrubField::connect(field, &NumberScrubField::valueChanged, field,[item, field, param](double){item->widgetUpdated(field, param);});
    return field;
}

void IntegerParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }
    else
    {
        NumberScrubField *field = static_cast<NumberScrubField*>(t_widget);
        field->setValue(value().toInt());
    }

}

QVariant IntegerParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<int>(static_cast<NumberScrubField*>(t_widget)->value());
}

void IntegerParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    m_impl->minimum = t_json.value("minimum").toInt();
    m_impl->maximum = t_json.value("maximum").toInt();
}

void IntegerParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    t_json.insert("minimum", m_impl->minimum);
    t_json.insert("maximum", m_impl->maximum);
}

} // namespace keira
