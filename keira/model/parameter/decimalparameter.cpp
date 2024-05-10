#include <QDoubleSpinBox>
#include <QLabel>
#include "decimalparameter.h"
#include "view/nodeitem.h"

namespace keira {

const QByteArray DecimalParameter::ParameterId = "decimal";

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

QWidget *DecimalParameter::createWidget(NodeItem *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }
    QDoubleSpinBox *spin = new QDoubleSpinBox();
    spin->setMaximumHeight(30);
    spin->setMaximum(m_impl->maximum);
    spin->setMinimum(m_impl->minimum);
    spin->setMinimumWidth(50);
    spin->setDecimals(m_impl->precision);

    spin->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    spin->setReadOnly(isReadOnly());
    if(isReadOnly())
        spin->setButtonSymbols(QAbstractSpinBox::NoButtons);

    const DecimalParameter *param = this;
    QDoubleSpinBox::connect(spin, &QDoubleSpinBox::editingFinished, spin,[item, spin, param](){item->widgetUpdated(spin, param);});
    QDoubleSpinBox::connect(spin, &QDoubleSpinBox::valueChanged, spin,[item, spin, param](double){item->widgetUpdated(spin, param);});
    return spin;
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
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(t_widget);
        spinBox->setValue(value().toDouble());
    }

}

QVariant DecimalParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<QDoubleSpinBox*>(t_widget)->value();
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
