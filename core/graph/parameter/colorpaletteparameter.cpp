
#include <QLabel>
#include "colorpaletteparameter.h"
#include "util/utils.h"
#include "gui/color/colorpalettewidget.h"

namespace photon {
const QByteArray ColorPaletteParameter::ParameterId = "colorPalette";

ColorPaletteParameter::ColorPaletteParameter() {}

ColorPaletteParameter::ColorPaletteParameter(const QByteArray &t_id, const QString &t_name, ColorPalette t_default, int connectionOptions):
    Parameter(ParameterId, t_id, t_name, QVariant::fromValue(t_default), connectionOptions)
{

}

ColorPaletteParameter::~ColorPaletteParameter()
{

}

QWidget *ColorPaletteParameter::createWidget(keira::NodeEditor *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }

    auto paletteWidget = new ColorPaletteWidget(value().value<ColorPalette>(),true);

    ColorPaletteParameter *param = const_cast<ColorPaletteParameter*>(this);

    ColorPaletteWidget::connect(paletteWidget, &ColorPaletteWidget::paletteUpdated,[paletteWidget, param](){param->setValue(param->updateValue(paletteWidget));});

    return paletteWidget;
}

void ColorPaletteParameter::updateWidget(QWidget *t_widget) const
{

    if(isReadOnly())
    {

    }
    else
    {
        ColorPaletteWidget *editor = static_cast<ColorPaletteWidget*>(t_widget);
        editor->setPalette(value().value<ColorPalette>());
    }


}

QVariant ColorPaletteParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();

    return QVariant::fromValue(static_cast<ColorPaletteWidget*>(t_widget)->palette());
}


void ColorPaletteParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    QJsonArray colorArray = t_json.value("value").toArray();

    ColorPalette palette;

    for(auto color : colorArray)
    {
        palette.append(jsonToColor(color.toObject()));
    }

    setValue(QVariant::fromValue(palette));

}

void ColorPaletteParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    QJsonArray colorArray;

    auto palette = value().value<ColorPalette>();

    for(auto color : palette)
    {
        colorArray.append(colorToJson(color));
    }


    t_json.insert("value",colorArray);

}

} // namespace photon
