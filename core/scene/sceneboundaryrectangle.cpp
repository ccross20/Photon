#include <QVBoxLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include "sceneboundaryrectangle.h"
#include "vector3edit.h"
#include "propertywidgets.h"
#include "tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneBoundaryRectangleEditorWidget::Impl
{
public:
    Impl();
    SceneBoundaryRectangle *rect = nullptr;
    PropertyForm *form;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QDoubleSpinBox *widthSpin;
    QDoubleSpinBox *heightSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneBoundaryRectangleEditorWidget::Impl::Impl()
{
    form = new PropertyForm;

    form->addSection("General");

    nameEdit = new QLineEdit;
    form->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return rect ? rect->tags() : QStringList(); },
        [this](const QStringList &tags){ if(rect) rect->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    form->addRow("Tags", tagEditor);

    widthSpin = new QDoubleSpinBox;
    widthSpin->setSuffix(QStringLiteral(" m"));
    widthSpin->setMinimum(0.1);
    widthSpin->setMaximum(200.0);
    form->addRow("Width", widthSpin);

    heightSpin = new QDoubleSpinBox;
    heightSpin->setSuffix(QStringLiteral(" m"));
    heightSpin->setMinimum(0.1);
    heightSpin->setMaximum(200.0);
    form->addRow("Height", heightSpin);
}

SceneBoundaryRectangleEditorWidget::SceneBoundaryRectangleEditorWidget(SceneBoundaryRectangle *t_rect, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_impl->form);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->rect = t_rect;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneBoundaryRectangleEditorWidget::setName);
    connect(m_impl->widthSpin, &QDoubleSpinBox::valueChanged, this, &SceneBoundaryRectangleEditorWidget::setWidth);
    connect(m_impl->heightSpin, &QDoubleSpinBox::valueChanged, this, &SceneBoundaryRectangleEditorWidget::setHeight);

    connect(t_rect, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_rect->name());
    m_impl->tagEditor->refresh();
    m_impl->widthSpin->setValue(t_rect->width());
    m_impl->heightSpin->setValue(t_rect->height());

    addHelperPropertyRows(m_impl->form, t_rect, this);

    m_impl->form->addSection("Transform");

    m_impl->positionEdit = new Vector3Edit(Vector3Edit::Distance);
    m_impl->form->addRow("Position", m_impl->positionEdit);
    m_impl->rotationEdit = new Vector3Edit(Vector3Edit::Angle);
    m_impl->form->addRow("Rotation", m_impl->rotationEdit);

    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneBoundaryRectangleEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneBoundaryRectangleEditorWidget::setRotation);
    connect(t_rect, &SceneObject::positionChanged, this, &SceneBoundaryRectangleEditorWidget::refreshTransform);
    connect(t_rect, &SceneObject::rotationChanged, this, &SceneBoundaryRectangleEditorWidget::refreshTransform);

    m_impl->positionEdit->setValue(t_rect->position());
    m_impl->rotationEdit->setValue(t_rect->rotation());
}

SceneBoundaryRectangleEditorWidget::~SceneBoundaryRectangleEditorWidget()
{
    delete m_impl;
}

void SceneBoundaryRectangleEditorWidget::setName(const QString &t_value)
{
    m_impl->rect->setName(t_value);
}

void SceneBoundaryRectangleEditorWidget::setWidth(double t_value)
{
    m_impl->rect->setWidth(float(t_value));
}

void SceneBoundaryRectangleEditorWidget::setHeight(double t_value)
{
    m_impl->rect->setHeight(float(t_value));
}

void SceneBoundaryRectangleEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->rect->setPosition(t_value);
}

void SceneBoundaryRectangleEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->rect->setRotation(t_value);
}

void SceneBoundaryRectangleEditorWidget::refreshTransform()
{
    QSignalBlocker pb(m_impl->positionEdit);
    QSignalBlocker rb(m_impl->rotationEdit);
    m_impl->positionEdit->setValue(m_impl->rect->position());
    m_impl->rotationEdit->setValue(m_impl->rect->rotation());
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneBoundaryRectangle::Impl
{
public:
    float width = 6.0f;
    float height = 6.0f;
};

SceneBoundaryRectangle::SceneBoundaryRectangle() : SceneHelperObject("boundaryrectangle"), m_impl(new Impl)
{
    setColor(QColor(100, 200, 220));
}

SceneBoundaryRectangle::~SceneBoundaryRectangle()
{
    delete m_impl;
}

QWidget *SceneBoundaryRectangle::createEditor()
{
    return new SceneBoundaryRectangleEditorWidget(this);
}

void SceneBoundaryRectangle::setWidth(float t_value)
{
    m_impl->width = t_value;
    emit metadataChanged(this);
}

void SceneBoundaryRectangle::setHeight(float t_value)
{
    m_impl->height = t_value;
    emit metadataChanged(this);
}

float SceneBoundaryRectangle::width() const
{
    return m_impl->width;
}

float SceneBoundaryRectangle::height() const
{
    return m_impl->height;
}

void SceneBoundaryRectangle::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneHelperObject::readFromJson(t_json, t_context);
    m_impl->width = t_json.value("width").toDouble(m_impl->width);
    m_impl->height = t_json.value("height").toDouble(m_impl->height);
}

void SceneBoundaryRectangle::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("width", m_impl->width);
    t_json.insert("height", m_impl->height);
}

} // namespace photon
