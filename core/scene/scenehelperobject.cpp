#include <QComboBox>
#include <QColorDialog>
#include <QFormLayout>
#include <QPushButton>
#include "scenehelperobject.h"

namespace photon {

class SceneHelperObject::Impl
{
public:
    QColor color = QColor(220, 200, 60);
    VisibilityMode visibilityMode = AlwaysVisible;
};

SceneHelperObject::SceneHelperObject(const QByteArray &t_typeId, SceneObject *t_parent)
    : SceneObject(t_typeId, t_parent), m_impl(new Impl)
{
}

QColor SceneHelperObject::color() const
{
    return m_impl->color;
}

void SceneHelperObject::setColor(const QColor &t_value)
{
    m_impl->color = t_value;
    emit metadataChanged(this);
}

SceneHelperObject::VisibilityMode SceneHelperObject::visibilityMode() const
{
    return m_impl->visibilityMode;
}

void SceneHelperObject::setVisibilityMode(VisibilityMode t_value)
{
    m_impl->visibilityMode = t_value;
    emit metadataChanged(this);
}

void SceneHelperObject::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneObject::readFromJson(t_json, t_context);
    if(t_json.contains("color"))
        m_impl->color = QColor(t_json.value("color").toString());
    m_impl->visibilityMode = t_json.value("visibility").toString() == "selected" ? SelectedOnly : AlwaysVisible;
}

void SceneHelperObject::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);
    t_json.insert("color", m_impl->color.name());
    t_json.insert("visibility", m_impl->visibilityMode == SelectedOnly ? "selected" : "always");
}

void addHelperPropertyRows(QFormLayout *t_form, SceneHelperObject *t_object, QWidget *t_editorParent)
{
    auto *colorButton = new QPushButton;
    auto updateSwatch = [colorButton](const QColor &c){
        colorButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
    };
    updateSwatch(t_object->color());
    QObject::connect(colorButton, &QPushButton::clicked, t_editorParent, [t_object, updateSwatch, t_editorParent](){
        const QColor c = QColorDialog::getColor(t_object->color(), t_editorParent, "Color");
        if(!c.isValid())
            return;
        t_object->setColor(c);
        updateSwatch(c);
    });
    t_form->addRow("Color", colorButton);

    auto *visibilityCombo = new QComboBox;
    visibilityCombo->addItems(QStringList() << "Always" << "When Selected");
    visibilityCombo->setCurrentIndex(t_object->visibilityMode() == SceneHelperObject::SelectedOnly ? 1 : 0);
    QObject::connect(visibilityCombo, &QComboBox::activated, t_editorParent, [t_object](int index){
        t_object->setVisibilityMode(index == 1 ? SceneHelperObject::SelectedOnly : SceneHelperObject::AlwaysVisible);
    });
    t_form->addRow("Visible", visibilityCombo);

    // Keeps the swatch live if the color changes from outside this editor
    // (e.g. another editor of the same object, or a future scripted change).
    QObject::connect(t_object, &SceneObject::metadataChanged, t_editorParent, [t_object, updateSwatch](){
        updateSwatch(t_object->color());
    });
}

} // namespace photon
