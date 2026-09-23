#include <QLabel>
#include "decimalparameter.h"
#include "integerparameter.h"
#include "booleanparameter.h"
#include "view/nodeeditor.h"
#include "numberscrubfield.h"

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
    // Mirror the hard bounds until setSoftMinimum/setSoftMaximum/setSoftRange
    // narrows them - hasSoftRangeSet tracks whether that's happened, so
    // createWidget() knows whether to push an explicit soft range onto the
    // field or just let it mirror the hard one (NumberScrubField's own
    // default behaviour).
    double softMinimum = std::numeric_limits<double>::lowest();
    double softMaximum = std::numeric_limits<double>::max();
    bool hasSoftRangeSet = false;
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

double DecimalParameter::minimum() const
{
    return m_impl->minimum;
}

double DecimalParameter::maximum() const
{
    return m_impl->maximum;
}

void DecimalParameter::setSoftMinimum(double t_min)
{
    m_impl->softMinimum = t_min;
    m_impl->hasSoftRangeSet = true;
}

void DecimalParameter::setSoftMaximum(double t_max)
{
    m_impl->softMaximum = t_max;
    m_impl->hasSoftRangeSet = true;
}

void DecimalParameter::setSoftRange(double t_min, double t_max)
{
    m_impl->softMinimum = t_min;
    m_impl->softMaximum = t_max;
    m_impl->hasSoftRangeSet = true;
}

double DecimalParameter::softMinimum() const
{
    return m_impl->hasSoftRangeSet ? m_impl->softMinimum : m_impl->minimum;
}

double DecimalParameter::softMaximum() const
{
    return m_impl->hasSoftRangeSet ? m_impl->softMaximum : m_impl->maximum;
}

bool DecimalParameter::hasSoftRange() const
{
    return m_impl->hasSoftRangeSet;
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
    photon::NumberScrubField *field = new photon::NumberScrubField();
    field->setMaximumHeight(30);
    field->setMinimumWidth(50);
    field->setIsInteger(false);
    field->setDecimals(m_impl->precision);
    field->setRange(m_impl->minimum, m_impl->maximum);
    if(m_impl->hasSoftRangeSet)
        field->setSoftRange(m_impl->softMinimum, m_impl->softMaximum);
    field->setValue(value().toDouble());
    field->setReadOnly(isReadOnly());

    const DecimalParameter *param = this;
    photon::NumberScrubField::connect(field, &photon::NumberScrubField::editingFinished, field,[item, field, param](){item->widgetUpdated(field, param);});
    photon::NumberScrubField::connect(field, &photon::NumberScrubField::valueChanged, field,[item, field, param](double){item->widgetUpdated(field, param);});
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
        photon::NumberScrubField *field = static_cast<photon::NumberScrubField*>(t_widget);
        field->setValue(value().toDouble());
    }

}

QVariant DecimalParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<photon::NumberScrubField*>(t_widget)->value();
}

void DecimalParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    m_impl->minimum = t_json.value("minimum").toDouble();
    m_impl->maximum = t_json.value("maximum").toDouble();
    m_impl->precision = t_json.value("precision").toInt();

    if(t_json.contains("softMinimum") || t_json.contains("softMaximum"))
    {
        m_impl->softMinimum = t_json.value("softMinimum").toDouble(m_impl->minimum);
        m_impl->softMaximum = t_json.value("softMaximum").toDouble(m_impl->maximum);
        m_impl->hasSoftRangeSet = true;
    }
}

void DecimalParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    t_json.insert("minimum", m_impl->minimum);
    t_json.insert("maximum", m_impl->maximum);
    t_json.insert("precision", static_cast<int>(m_impl->precision));

    if(m_impl->hasSoftRangeSet)
    {
        t_json.insert("softMinimum", m_impl->softMinimum);
        t_json.insert("softMaximum", m_impl->softMaximum);
    }
}

} // namespace keira
