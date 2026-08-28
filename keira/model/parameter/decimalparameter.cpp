#include <QLabel>
#include "decimalparameter.h"
#include "integerparameter.h"
#include "booleanparameter.h"
#include "view/nodeeditor.h"
#include "view/numberscrubfield.h"

namespace keira {

const QByteArray DecimalParameter::ParameterId = "decimal";

bool DecimalParameter::acceptsConnectionFrom(const Parameter *source) const
{
    return Parameter::acceptsConnectionFrom(source)
        || source->typeId() == IntegerParameter::ParameterId
        // A boolean reads as 0 or 1 - setValue()'s toDouble() does the
        // conversion, so a gate or comparison can drive a numeric input
        // directly without a converter node in between.
        || source->typeId() == BooleanParameter::ParameterId;
}

class DecimalParameter::Impl
{
public:
    double minimum = std::numeric_limits<double>::lowest();
    double maximum = std::numeric_limits<double>::max();
    uint precision = 4;
};

DecimalParameter::DecimalParameter() : Parameter(),m_impl(new Impl)
{

}

DecimalParameter::DecimalParameter(const QByteArray &t_id, const QString &t_name, double t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{

}

DecimalParameter::~DecimalParameter()
{
    delete m_impl;
}

void DecimalParameter::setMinimum(double t_min)
{
    m_impl->minimum = t_min;
}

void DecimalParameter::setMaximum(double t_max)
{
    m_impl->maximum = t_max;
}

void DecimalParameter::setPrecision(uint t_precision)
{
    m_impl->precision = t_precision;
}

void DecimalParameter::setValue(const QVariant &t_value)
{
    double val = std::max(std::min(t_value.toDouble(), m_impl->maximum),m_impl->minimum);

    Parameter::setValue(val);
}

QWidget *DecimalParameter::createWidget(NodeEditor *item) const
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
    field->setIsInteger(false);
    field->setDecimals(m_impl->precision);
    field->setRange(m_impl->minimum, m_impl->maximum);
    field->setValue(value().toDouble());
    field->setReadOnly(isReadOnly());

    const DecimalParameter *param = this;
    NumberScrubField::connect(field, &NumberScrubField::editingFinished, field,[item, field, param](){item->widgetUpdated(field, param);});
    NumberScrubField::connect(field, &NumberScrubField::valueChanged, field,[item, field, param](double){item->widgetUpdated(field, param);});
    return field;
}

void DecimalParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }
    else
    {
        NumberScrubField *field = static_cast<NumberScrubField*>(t_widget);
        field->setValue(value().toDouble());
    }

}

QVariant DecimalParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<NumberScrubField*>(t_widget)->value();
}

void DecimalParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    m_impl->minimum = t_json.value("minimum").toDouble();
    m_impl->maximum = t_json.value("maximum").toDouble();
    m_impl->precision = t_json.value("precision").toInt();
}

void DecimalParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    t_json.insert("minimum", m_impl->minimum);
    t_json.insert("maximum", m_impl->maximum);
    t_json.insert("precision", static_cast<int>(m_impl->precision));
}

} // namespace keira
