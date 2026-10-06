#include <QVBoxLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QSignalBlocker>
#include "scenesurface.h"
#include "vector3edit.h"
#include "propertywidgets.h"
#include "tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneSurfaceEditorWidget::Impl
{
public:
    explicit Impl(SceneSurface *surface);
    SceneSurface *surface = nullptr;
    PropertyForm *form;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QDoubleSpinBox *widthSpin;
    QDoubleSpinBox *heightSpin;
    QPushButton *colorButton;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneSurfaceEditorWidget::Impl::Impl(SceneSurface *t_surface) : surface(t_surface)
{
    form = new PropertyForm;

    form->addSection("General");

    nameEdit = new QLineEdit;
    form->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return surface ? surface->tags() : QStringList(); },
        [this](const QStringList &tags){ if(surface) surface->setTags(tags); },
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

    form->addSection("Style");

    colorButton = new QPushButton;
    form->addRow("Color", colorButton);

    form->addRow("Display Cull", PropertyWidgets::createOptions({"None", "Back", "Front"}, int(surface->displayCull()), {},
        [this](int index){ if(surface) surface->setDisplayCull(static_cast<SceneSurface::DisplayCull>(index)); }));

    form->addSection("Transform");

    positionEdit = new Vector3Edit(Vector3Edit::Distance);
    form->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit(Vector3Edit::Angle);
    form->addRow("Rotation", rotationEdit);
}

SceneSurfaceEditorWidget::SceneSurfaceEditorWidget(SceneSurface *t_surface, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl(t_surface))
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_impl->form);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneSurfaceEditorWidget::setName);
    connect(m_impl->widthSpin, &QDoubleSpinBox::valueChanged, this, &SceneSurfaceEditorWidget::setWidth);
    connect(m_impl->heightSpin, &QDoubleSpinBox::valueChanged, this, &SceneSurfaceEditorWidget::setHeight);
    connect(m_impl->colorButton, &QPushButton::clicked, this, &SceneSurfaceEditorWidget::chooseColor);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneSurfaceEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneSurfaceEditorWidget::setRotation);

    connect(t_surface, &SceneObject::positionChanged, this, &SceneSurfaceEditorWidget::refreshTransform);
    connect(t_surface, &SceneObject::rotationChanged, this, &SceneSurfaceEditorWidget::refreshTransform);
    // Keeps the tag row live if a tag is added/removed from outside this
    // editor - e.g. dropped onto this object's row in the Project panel.
    connect(t_surface, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_surface->name());
    m_impl->tagEditor->refresh();
    m_impl->widthSpin->setValue(t_surface->surfaceWidth());
    m_impl->heightSpin->setValue(t_surface->surfaceHeight());
    m_impl->positionEdit->setValue(t_surface->position());
    m_impl->rotationEdit->setValue(t_surface->rotation());

    const QColor c = t_surface->color();
    m_impl->colorButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
}

SceneSurfaceEditorWidget::~SceneSurfaceEditorWidget()
{
    delete m_impl;
}

void SceneSurfaceEditorWidget::setName(const QString &t_value)
{
    m_impl->surface->setName(t_value);
}

void SceneSurfaceEditorWidget::setWidth(double t_value)
{
    m_impl->surface->setSurfaceWidth(float(t_value));
}

void SceneSurfaceEditorWidget::setHeight(double t_value)
{
    m_impl->surface->setSurfaceHeight(float(t_value));
}

void SceneSurfaceEditorWidget::chooseColor()
{
    const QColor c = QColorDialog::getColor(m_impl->surface->color(), this, "Surface Color");
    if (!c.isValid())
        return;
    m_impl->surface->setColor(c);
    m_impl->colorButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
}

void SceneSurfaceEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->surface->setPosition(t_value);
}

void SceneSurfaceEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->surface->setRotation(t_value);
}

void SceneSurfaceEditorWidget::refreshTransform()
{
    QSignalBlocker pb(m_impl->positionEdit);
    QSignalBlocker rb(m_impl->rotationEdit);
    m_impl->positionEdit->setValue(m_impl->surface->position());
    m_impl->rotationEdit->setValue(m_impl->surface->rotation());
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneSurface::Impl
{
public:
    float width = 12.0f;
    float height = 6.0f;
    QColor color = QColor(180, 180, 185);
    DisplayCull displayCull = CullNone;
};

namespace {

const QStringList &displayCullNames()
{
    static const QStringList names{"none", "back", "front"};
    return names;
}

} // namespace

SceneSurface::SceneSurface() : SceneObject("surface"), m_impl(new Impl)
{
}

SceneSurface::~SceneSurface()
{
    delete m_impl;
}

QWidget *SceneSurface::createEditor()
{
    return new SceneSurfaceEditorWidget(this);
}

void SceneSurface::setSurfaceWidth(float t_value)
{
    m_impl->width = t_value;
    emit metadataChanged(this);
}

void SceneSurface::setSurfaceHeight(float t_value)
{
    m_impl->height = t_value;
    emit metadataChanged(this);
}

void SceneSurface::setColor(const QColor &t_value)
{
    m_impl->color = t_value;
    emit metadataChanged(this);
}

void SceneSurface::setDisplayCull(DisplayCull t_value)
{
    if (m_impl->displayCull == t_value)
        return;
    m_impl->displayCull = t_value;
    emit metadataChanged(this);
}

SceneSurface::DisplayCull SceneSurface::displayCull() const
{
    return m_impl->displayCull;
}

float SceneSurface::surfaceWidth() const
{
    return m_impl->width;
}

float SceneSurface::surfaceHeight() const
{
    return m_impl->height;
}

QColor SceneSurface::color() const
{
    return m_impl->color;
}

void SceneSurface::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneObject::readFromJson(t_json, t_context);
    m_impl->width = t_json.value("width").toDouble(m_impl->width);
    m_impl->height = t_json.value("height").toDouble(m_impl->height);
    if (t_json.contains("color"))
        m_impl->color = QColor(t_json.value("color").toString());
    const int cull = displayCullNames().indexOf(t_json.value("displayCull").toString());
    m_impl->displayCull = cull < 0 ? CullNone : static_cast<DisplayCull>(cull);
}

void SceneSurface::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);
    t_json.insert("width", m_impl->width);
    t_json.insert("height", m_impl->height);
    t_json.insert("color", m_impl->color.name());
    t_json.insert("displayCull", displayCullNames().at(m_impl->displayCull));
}

} // namespace photon
