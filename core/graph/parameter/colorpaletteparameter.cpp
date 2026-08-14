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

QWidget *ColorPaletteParameter::createWidget(keira::NodeEditor *) const
{
    // isReadOnly() here just means "can't receive a graph connection" (no
    // AllowSingleInput/AllowMultipleInput bit) - an output-only palette (e.g.
    // a generator node's result) still needs to actually show its colors, so
    // this always builds the real widget and only withholds edit affordances
    // (the "Add" button, click-to-edit) via isEditable, rather than falling
    // back to an empty label.
    auto paletteWidget = new ColorPaletteWidget(value().value<ColorPalette>(), !isReadOnly());

    ColorPaletteParameter *param = const_cast<ColorPaletteParameter*>(this);

    if(!isReadOnly())
        ColorPaletteWidget::connect(paletteWidget, &ColorPaletteWidget::paletteUpdated,[paletteWidget, param](){param->setValue(param->updateValue(paletteWidget));});

    return paletteWidget;
}

void ColorPaletteParameter::updateWidget(QWidget *t_widget) const
{
    static_cast<ColorPaletteWidget*>(t_widget)->setPalette(value().value<ColorPalette>());
}

QVariant ColorPaletteParameter::updateValue(QWidget *t_widget) const
{
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
