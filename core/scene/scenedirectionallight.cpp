#include <algorithm>
#include <QJsonObject>
#include <QLineEdit>
#include <QSignalBlocker>
#include "vector3edit.h"
#include <QVBoxLayout>
#include "scenedirectionallight.h"
#include "propertywidgets.h"
#include "tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneDirectionalLight::Impl
{
public:
    QColor color = QColor(255, 255, 255);
    double intensity = 0.3;
};

SceneDirectionalLight::SceneDirectionalLight() : SceneObject("directionallight"), m_impl(new Impl)
{
}

SceneDirectionalLight::~SceneDirectionalLight()
{
    delete m_impl;
}

void SceneDirectionalLight::setColor(const QColor &t_value)
{
    if (m_impl->color == t_value)
        return;
    m_impl->color = t_value;
    emit metadataChanged(this);
}

void SceneDirectionalLight::setIntensity(double t_value)
{
    const double value = std::clamp(t_value, 0.0, 1.0);
    if (m_impl->intensity == value)
        return;
    m_impl->intensity = value;
    emit metadataChanged(this);
}

QColor SceneDirectionalLight::color() const
{
    return m_impl->color;
}

double SceneDirectionalLight::intensity() const
{
    return m_impl->intensity;
}

QWidget *SceneDirectionalLight::createEditor()
{
    auto *editor = new QWidget;
    auto *outer = new QVBoxLayout(editor);
    outer->setContentsMargins(0, 0, 0, 0);
    editor->setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    auto *form = new PropertyForm;
    outer->addWidget(form);

    form->addSection("General");

    auto *nameEdit = new QLineEdit(name());
    QObject::connect(nameEdit, &QLineEdit::textEdited, this, &SceneObject::setName);
    form->addRow("Name", nameEdit);

    auto *tagEditor = new TagEditorWidget(
        [this](){ return tags(); },
        [this](const QStringList &t_tags){ setTags(t_tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    // Keeps the tag row live if a tag is dropped onto this object's row in
    // the Project panel.
    QObject::connect(this, &SceneObject::metadataChanged, tagEditor, &TagEditorWidget::refresh);
    tagEditor->refresh();
    form->addRow("Tags", tagEditor);

    form->addSection("Light");

    form->addRow("Color", PropertyWidgets::createColor(m_impl->color, {},
        [this](const QColor &c){ setColor(c); }));
    form->addRow("Intensity", PropertyWidgets::createNumber(m_impl->intensity,
        {{PropertyWidgets::MetaMinimum, 0.0}, {PropertyWidgets::MetaMaximum, 1.0}},
        [this](double v){ setIntensity(v); }));

    // Rotation aims the light; position only places its arrow.
    form->addSection("Transform");

    auto *positionEdit = new Vector3Edit(Vector3Edit::Distance);
    positionEdit->setValue(position());
    QObject::connect(positionEdit, &Vector3Edit::valueChanged, this, &SceneObject::setPosition);
    form->addRow("Position", positionEdit);

    auto *rotationEdit = new Vector3Edit(Vector3Edit::Angle);
    rotationEdit->setValue(rotation());
    QObject::connect(rotationEdit, &Vector3Edit::valueChanged, this, &SceneObject::setRotation);
    form->addRow("Rotation", rotationEdit);

    // Keep the fields live while the gizmo moves or turns the light.
    auto refreshTransform = [this, positionEdit, rotationEdit](){
        QSignalBlocker pb(positionEdit);
        QSignalBlocker rb(rotationEdit);
        positionEdit->setValue(position());
        rotationEdit->setValue(rotation());
    };
    QObject::connect(this, &SceneObject::positionChanged, editor, refreshTransform);
    QObject::connect(this, &SceneObject::rotationChanged, editor, refreshTransform);

    return editor;
}

void SceneDirectionalLight::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneObject::readFromJson(t_json, t_context);
    if (t_json.contains("color"))
        m_impl->color = QColor(t_json.value("color").toString());
    m_impl->intensity = std::clamp(t_json.value("intensity").toDouble(m_impl->intensity), 0.0, 1.0);
}

void SceneDirectionalLight::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);
    t_json.insert("color", m_impl->color.name());
    t_json.insert("intensity", m_impl->intensity);
}

} // namespace photon
