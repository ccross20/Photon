#include <QUuid>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QPushButton>
#include "colorresource.h"
#include "gui/resourceeditorwidget.h"
#include "propertywidgets.h"
#include "color/colorselectordialog.h"

namespace photon {

class ColorResource::Impl
{
public:
    QByteArray uniqueId;
    QString name;
    QColor color = Qt::white;
};

ColorResource::ColorResource() : m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

ColorResource::~ColorResource()
{
    delete m_impl;
}

QByteArray ColorResource::uniqueId() const
{
    return m_impl->uniqueId;
}

QString ColorResource::name() const
{
    return m_impl->name;
}

void ColorResource::setName(const QString &t_name)
{
    if(m_impl->name == t_name)
        return;
    m_impl->name = t_name;
    notifyResourceChanged();
}

QColor ColorResource::color() const
{
    return m_impl->color;
}

void ColorResource::setColor(const QColor &t_color)
{
    if(m_impl->color == t_color)
        return;
    m_impl->color = t_color;
    notifyResourceChanged();
}

QWidget *ColorResource::createResourceEditor()
{
    auto *container = new QWidget;
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new ResourceEditorWidget(this));

    auto *form = new PropertyForm;

    // The app's own colour picker (also used by the saved-colours strip and
    // the colour palette editor) rather than the OS-native QColorDialog, so
    // this matches the rest of the app's colour-picking UI.
    auto *swatch = new QPushButton;
    swatch->setMaximumHeight(24);
    swatch->setMinimumWidth(50);
    swatch->setCursor(Qt::PointingHandCursor);

    auto paint = [swatch](const QColor &c){
        swatch->setStyleSheet(QString("QPushButton { background-color: %1; border: 1px solid #2b2b2b;"
                                      " border-radius: 3px; }"
                                      "QPushButton:hover { border: 1px solid #777777; }")
                                  .arg(c.name()));
    };
    paint(m_impl->color);

    connect(swatch, &QPushButton::clicked, this, [this, paint](){
        auto *dialog = new ColorSelectorDialog(m_impl->color);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
        dialog->raise();
        dialog->activateWindow();

        connect(dialog, &ColorSelectorDialog::selectionChanged, this, [this, paint](QColor c){
            setColor(c);
            paint(c);
        });
    });

    form->addRow("Color", swatch);
    layout->addWidget(form);
    layout->addStretch(1);

    return container;
}

void ColorResource::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    if(t_json.contains("uniqueId"))
        m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    if(t_json.contains("color"))
        m_impl->color = QColor(t_json.value("color").toString());
    readResourceJson(t_json);
}

void ColorResource::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString{m_impl->uniqueId});
    t_json.insert("color", m_impl->color.name(QColor::HexArgb));
    writeResourceJson(t_json);
}

} // namespace photon
