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

    // The default checkable QPushButton relies on the platform style to show
    // checked vs. unchecked, which on macOS is just a subtle shade-of-gray
    // difference that's easy to miss at a glance. Give the two states
    // distinct colors (and a text label) instead of leaning on style alone.
    button->setStyleSheet(
        "QPushButton { background-color: #4a4a4a; border: 1px solid #2b2b2b; border-radius: 3px; color: #cfcfcf; }"
        "QPushButton:hover { border: 1px solid #777777; }"
        "QPushButton:checked { background-color: #3fa66c; border: 1px solid #2c7d4f; color: #ffffff; font-weight: bold; }"
        "QPushButton:checked:hover { background-color: #48ba79; }"
    );
    updateButtonText(button);

    const BooleanParameter *param = this;
    QPushButton::connect(button, &QPushButton::toggled, button,[item, button, param](bool value){updateButtonText(button); item->widgetUpdated(button, param);});
    return button;
}

void BooleanParameter::updateButtonText(QPushButton *button)
{
    button->setText(button->isChecked() ? "On" : "Off");
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
        updateButtonText(spinBox);
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
