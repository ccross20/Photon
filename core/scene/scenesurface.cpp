#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QSignalBlocker>
#include "scenesurface.h"
#include "gui/vector3edit.h"

namespace photon {

class SceneSurfaceEditorWidget::Impl
{
public:
    Impl();
    SceneSurface *surface;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    QDoubleSpinBox *widthSpin;
    QDoubleSpinBox *heightSpin;
    QPushButton *colorButton;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneSurfaceEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    widthSpin = new QDoubleSpinBox;
    widthSpin->setMinimum(0.1);
    widthSpin->setMaximum(200.0);
    formLayout->addRow("Width", widthSpin);

    heightSpin = new QDoubleSpinBox;
    heightSpin->setMinimum(0.1);
    heightSpin->setMaximum(200.0);
    formLayout->addRow("Height", heightSpin);

    colorButton = new QPushButton;
    formLayout->addRow("Color", colorButton);

    positionEdit = new Vector3Edit;
    formLayout->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit;
    formLayout->addRow("Rotation", rotationEdit);
}

SceneSurfaceEditorWidget::SceneSurfaceEditorWidget(SceneSurface *t_surface, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    setLayout(m_impl->formLayout);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->surface = t_surface;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneSurfaceEditorWidget::setName);
    connect(m_impl->widthSpin, &QDoubleSpinBox::valueChanged, this, &SceneSurfaceEditorWidget::setWidth);
    connect(m_impl->heightSpin, &QDoubleSpinBox::valueChanged, this, &SceneSurfaceEditorWidget::setHeight);
    connect(m_impl->colorButton, &QPushButton::clicked, this, &SceneSurfaceEditorWidget::chooseColor);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneSurfaceEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneSurfaceEditorWidget::setRotation);

    connect(t_surface, &SceneObject::positionChanged, this, &SceneSurfaceEditorWidget::refreshTransform);
    connect(t_surface, &SceneObject::rotationChanged, this, &SceneSurfaceEditorWidget::refreshTransform);

    m_impl->nameEdit->setText(t_surface->name());
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
};

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
}

void SceneSurface::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);
    t_json.insert("width", m_impl->width);
    t_json.insert("height", m_impl->height);
    t_json.insert("color", m_impl->color.name());
}

} // namespace photon
