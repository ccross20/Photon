#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QVBoxLayout>
#include "colorpaletteresource.h"
#include "gui/resourceeditorwidget.h"
#include "gui/color/colorpalettewidget.h"

namespace photon {

class ColorPaletteResource::Impl
{
public:
    QByteArray uniqueId;
    QString name;
    ColorPalette palette;
};

ColorPaletteResource::ColorPaletteResource() : m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

ColorPaletteResource::~ColorPaletteResource()
{
    delete m_impl;
}

QByteArray ColorPaletteResource::uniqueId() const
{
    return m_impl->uniqueId;
}

QString ColorPaletteResource::name() const
{
    return m_impl->name;
}

void ColorPaletteResource::setName(const QString &t_name)
{
    if(m_impl->name == t_name)
        return;
    m_impl->name = t_name;
    notifyResourceChanged();
}

ColorPalette ColorPaletteResource::palette() const
{
    return m_impl->palette;
}

void ColorPaletteResource::setPalette(const ColorPalette &t_palette)
{
    if(m_impl->palette == t_palette)
        return;
    m_impl->palette = t_palette;
    notifyResourceChanged();
}

QWidget *ColorPaletteResource::createResourceEditor()
{
    auto *container = new QWidget;
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new ResourceEditorWidget(this));

    auto *paletteWidget = new ColorPaletteWidget(m_impl->palette, true);
    connect(paletteWidget, &ColorPaletteWidget::paletteUpdated, this, [this, paletteWidget](){
        setPalette(paletteWidget->palette());
    });
    layout->addWidget(paletteWidget);
    layout->addStretch(1);

    return container;
}

void ColorPaletteResource::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    if(t_json.contains("uniqueId"))
        m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    if(t_json.contains("colors"))
    {
        m_impl->palette.clear();
        for(const auto &value : t_json.value("colors").toArray())
            m_impl->palette.append(QColor(value.toString()));
    }
    readResourceJson(t_json);
}

void ColorPaletteResource::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString{m_impl->uniqueId});
    QJsonArray colorArray;
    for(const auto &color : m_impl->palette)
        colorArray.append(color.name(QColor::HexArgb));
    t_json.insert("colors", colorArray);
    writeResourceJson(t_json);
}

} // namespace photon
