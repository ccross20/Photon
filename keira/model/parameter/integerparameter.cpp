#include <QLabel>
#include "integerparameter.h"
#include "decimalparameter.h"
#include "booleanparameter.h"
#include "view/nodeeditor.h"
#include "numberscrubfield.h"

namespace keira {

const QByteArray IntegerParameter::ParameterId = "integer";

bool IntegerParameter::acceptsConnectionFrom(const Parameter *source) const
{
    return Parameter::acceptsConnectionFrom(source)
        || source->typeId() == DecimalParameter::ParameterId
        // A boolean reads as 0 or 1 - readers go through value().toInt(),
        // which does the conversion, so a gate or comparison can drive a
        // numeric input directly without a converter node in between.
        || source->typeId() == BooleanParameter::ParameterId;
}

class IntegerParameter::Impl
{
public:
    int minimum = std::numeric_limits<int>::lowest();
    int maximum = std::numeric_limits<int>::max();
    // See DecimalParameter::Impl's matching fields for the mirror-until-set
    // contract this implements.
    int softMinimum = std::numeric_limits<int>::lowest();
    int softMaximum = std::numeric_limits<int>::max();
    bool hasSoftRangeSet = false;
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

int IntegerParameter::minimum() const
{
    return m_impl->minimum;
}

int IntegerParameter::maximum() const
{
    return m_impl->maximum;
}

void IntegerParameter::setSoftMinimum(int t_min)
{
    m_impl->softMinimum = t_min;
    m_impl->hasSoftRangeSet = true;
}

void IntegerParameter::setSoftMaximum(int t_max)
{
    m_impl->softMaximum = t_max;
    m_impl->hasSoftRangeSet = true;
}

void IntegerParameter::setSoftRange(int t_min, int t_max)
{
    m_impl->softMinimum = t_min;
    m_impl->softMaximum = t_max;
    m_impl->hasSoftRangeSet = true;
}

int IntegerParameter::softMinimum() const
{
    return m_impl->hasSoftRangeSet ? m_impl->softMinimum : m_impl->minimum;
}

int IntegerParameter::softMaximum() const
{
    return m_impl->hasSoftRangeSet ? m_impl->softMaximum : m_impl->maximum;
}

bool IntegerParameter::hasSoftRange() const
{
    return m_impl->hasSoftRangeSet;
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
    photon::NumberScrubField *field = new photon::NumberScrubField();
    field->setMaximumHeight(30);
    field->setMinimumWidth(50);
    field->setIsInteger(true);
    field->setRange(m_impl->minimum, m_impl->maximum);
    if(m_impl->hasSoftRangeSet)
        field->setSoftRange(m_impl->softMinimum, m_impl->softMaximum);
    field->setValue(value().toInt());
    field->setReadOnly(isReadOnly());

    const IntegerParameter *param = this;
    photon::NumberScrubField::connect(field, &photon::NumberScrubField::editingFinished, field,[item, field, param](){item->widgetUpdated(field, param);});
    photon::NumberScrubField::connect(field, &photon::NumberScrubField::valueChanged, field,[item, field, param](double){item->widgetUpdated(field, param);});
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
        photon::NumberScrubField *field = static_cast<photon::NumberScrubField*>(t_widget);
        field->setValue(value().toInt());
    }

}

QVariant IntegerParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<int>(static_cast<photon::NumberScrubField*>(t_widget)->value());
}

void IntegerParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    m_impl->minimum = t_json.value("minimum").toInt();
    m_impl->maximum = t_json.value("maximum").toInt();

    if(t_json.contains("softMinimum") || t_json.contains("softMaximum"))
    {
        m_impl->softMinimum = t_json.value("softMinimum").toInt(m_impl->minimum);
        m_impl->softMaximum = t_json.value("softMaximum").toInt(m_impl->maximum);
        m_impl->hasSoftRangeSet = true;
    }
}

void IntegerParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    t_json.insert("minimum", m_impl->minimum);
    t_json.insert("maximum", m_impl->maximum);

    if(m_impl->hasSoftRangeSet)
    {
        t_json.insert("softMinimum", m_impl->softMinimum);
        t_json.insert("softMaximum", m_impl->softMaximum);
    }
}

} // namespace keira
