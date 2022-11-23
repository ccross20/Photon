#include <QSpinBox>
#include <QLabel>
#include "integerparameter.h"
#include "view/nodeitem.h"

namespace keira {

const QByteArray IntegerParameter::ParameterId = "integer";

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

QWidget *IntegerParameter::createWidget(NodeItem *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }
    QSpinBox *spin = new QSpinBox();
    spin->setMaximumHeight(30);
    spin->setMaximum(m_impl->maximum);
    spin->setMinimum(m_impl->minimum);
    spin->setMinimumWidth(50);

    spin->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    spin->setReadOnly(isReadOnly());
    if(isReadOnly())
        spin->setButtonSymbols(QAbstractSpinBox::NoButtons);

    const IntegerParameter *param = this;
    QSpinBox::connect(spin, &QSpinBox::editingFinished, spin,[item, spin, param](){item->widgetUpdated(spin, param);});
    QSpinBox::connect(spin, &QSpinBox::valueChanged, spin,[item, spin, param](int){item->widgetUpdated(spin, param);});
    return spin;
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
        QSpinBox *spinBox = static_cast<QSpinBox*>(t_widget);
        spinBox->setValue(value().toInt());
    }

}

QVariant IntegerParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<QSpinBox*>(t_widget)->value();
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
