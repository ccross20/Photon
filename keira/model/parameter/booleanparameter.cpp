#include <QPushButton>
#include <QLabel>
#include "booleanparameter.h"
#include "integerparameter.h"
#include "decimalparameter.h"
#include "view/nodeeditor.h"

namespace keira {

const QByteArray BooleanParameter::ParameterId = "boolean";

bool BooleanParameter::acceptsConnectionFrom(const Parameter *source) const
{
    return Parameter::acceptsConnectionFrom(source)
        || source->typeId() == IntegerParameter::ParameterId
        || source->typeId() == DecimalParameter::ParameterId;
}

void BooleanParameter::setValue(const QVariant &t_value)
{
    // Convert here rather than leaving the number stored raw: every reader
    // (and the checkbox widget) goes through value().toBool(), which is a
    // not-equal-to-zero test - so a negative number would come back true.
    // Above zero is the rule, so -1 is false.
    //
    // Only actual numeric types are converted. A bool passes through as-is,
    // and anything else keeps the base class's behaviour rather than being
    // silently coerced through a meaningless double.
    switch(t_value.typeId())
    {
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
        Parameter::setValue(t_value.toDouble() > 0.0);
        return;
    default:
        break;
    }

    Parameter::setValue(t_value);
}


BooleanParameter::BooleanParameter() : Parameter()
{

}

BooleanParameter::BooleanParameter(const QByteArray &t_id, const QString &t_name, bool t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{

}

BooleanParameter::~BooleanParameter()
{
}



QWidget *BooleanParameter::createWidget(NodeEditor *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }
    QPushButton *button = new QPushButton();
    button->setMaximumHeight(30);
    button->setMinimumWidth(50);
    button->setCheckable(true);
    button->setChecked(value().toBool());

    button->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));


    const BooleanParameter *param = this;
    QPushButton::connect(button, &QPushButton::toggled, button,[item, button, param](bool value){item->widgetUpdated(button, param);});
    return button;
}

void BooleanParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }
    else
    {
        QPushButton *spinBox = static_cast<QPushButton*>(t_widget);
        spinBox->setChecked(value().toBool());
    }

}

QVariant BooleanParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<QPushButton*>(t_widget)->isChecked();
}

void BooleanParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);
}

void BooleanParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);
}

} // namespace keira
