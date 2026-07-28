#include <QLabel>
#include "gradientparameter.h"
#include "view/nodeeditor.h"
#include "gui/color/gradientwidget.h"

namespace photon {

const QByteArray GradientParameter::ParameterId = "gradient";

GradientParameter::GradientParameter() : Parameter()
{
}

GradientParameter::GradientParameter(const QByteArray &t_id, const QString &t_name, Gradient t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, QVariant::fromValue(t_default), connectionOptions)
{
}

QWidget *GradientParameter::createWidget(keira::NodeEditor *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }

    const GradientParameter *param = this;

    GradientWidget *widget = new GradientWidget(value().value<Gradient>());
    GradientWidget::connect(widget, &GradientWidget::gradientChanged, widget,
                            [item, widget, param](const Gradient &){ item->widgetUpdated(widget, param); });
    return widget;
}

void GradientParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
        return;

    GradientWidget *widget = static_cast<GradientWidget*>(t_widget);
    widget->setGradient(value().value<Gradient>());
}

QVariant GradientParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return value();

    return QVariant::fromValue(static_cast<GradientWidget*>(t_widget)->gradient());
}

void GradientParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    Gradient gradient;
    gradient.readFromJson(t_json.value("value").toObject());
    setValue(QVariant::fromValue(gradient));
}

void GradientParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    QJsonObject obj;
    value().value<Gradient>().writeToJson(obj);
    t_json.insert("value", obj);
}

} // namespace photon
