#include <QSpinBox>
#include <QLabel>
#include "colorparameter.h"
#include "view/nodeitem.h"
#include "gui/color/colorwheelswatch.h"

namespace photon {

const QByteArray ColorParameter::ParameterId = "color";

class ColorParameter::Impl
{
public:

};

ColorParameter::ColorParameter() : Parameter(),m_impl(new Impl)
{

}

ColorParameter::ColorParameter(const QByteArray &t_id, const QString &t_name, QColor t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{

}

ColorParameter::~ColorParameter()
{
    delete m_impl;
}


QWidget *ColorParameter::createWidget(keira::NodeItem *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }

    const ColorParameter *param = this;

    ColorWheelSwatch *swatch = new ColorWheelSwatch();
    swatch->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));
    ColorWheelSwatch::connect(swatch, &ColorWheelSwatch::colorChanged, swatch,[item, swatch, param](QColor){item->widgetUpdated(swatch, param);});
    /*
    QSpinBox *spin = new QSpinBox();
    spin->setMaximumHeight(30);
    spin->setMaximum(m_impl->maximum);
    spin->setMinimum(m_impl->minimum);
    spin->setMinimumWidth(50);

    spin->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    spin->setReadOnly(isReadOnly());
    if(isReadOnly())
        spin->setButtonSymbols(QAbstractSpinBox::NoButtons);

    const ColorParameter *param = this;
    QSpinBox::connect(spin, &QSpinBox::editingFinished, spin,[item, spin, param](){item->widgetUpdated(spin, param);});
    QSpinBox::connect(spin, &QSpinBox::valueChanged, spin,[item, spin, param](int){item->widgetUpdated(spin, param);});
    return spin;
    */

    return swatch;
}

void ColorParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }
    else
    {
        ColorWheelSwatch *swatch = static_cast<ColorWheelSwatch*>(t_widget);
        swatch->setColor(value().value<QColor>());
    }

}

QVariant ColorParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<ColorWheelSwatch*>(t_widget)->color();
}

void ColorParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

}

void ColorParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

}

} // namespace keira
