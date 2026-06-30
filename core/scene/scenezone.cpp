#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QSignalBlocker>
#include <QJsonObject>
#include <QMatrix4x4>
#include <cmath>
#include "scenezone.h"
#include "gui/vector3edit.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneZoneEditorWidget::Impl
{
public:
    Impl();
    SceneZone *zone;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    Vector3Edit *sizeEdit;
    QPushButton *colorButton;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneZoneEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    sizeEdit = new Vector3Edit;
    formLayout->addRow("Size", sizeEdit);

    colorButton = new QPushButton;
    formLayout->addRow("Color", colorButton);

    positionEdit = new Vector3Edit;
    formLayout->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit;
    formLayout->addRow("Rotation", rotationEdit);
}

SceneZoneEditorWidget::SceneZoneEditorWidget(SceneZone *t_zone, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    setLayout(m_impl->formLayout);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->zone = t_zone;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneZoneEditorWidget::setName);
    connect(m_impl->sizeEdit, &Vector3Edit::valueChanged, this, &SceneZoneEditorWidget::setSize);
    connect(m_impl->colorButton, &QPushButton::clicked, this, &SceneZoneEditorWidget::chooseColor);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneZoneEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneZoneEditorWidget::setRotation);

    connect(t_zone, &SceneObject::positionChanged, this, &SceneZoneEditorWidget::refreshTransform);
    connect(t_zone, &SceneObject::rotationChanged, this, &SceneZoneEditorWidget::refreshTransform);
    // Size changes (e.g. the scale gizmo) emit metadataChanged — keep the field live.
    connect(t_zone, &SceneObject::metadataChanged, this, &SceneZoneEditorWidget::refreshSize);

    m_impl->nameEdit->setText(t_zone->name());
    m_impl->sizeEdit->setValue(t_zone->size());
    m_impl->positionEdit->setValue(t_zone->position());
    m_impl->rotationEdit->setValue(t_zone->rotation());

    const QColor c = t_zone->color();
    m_impl->colorButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
}

SceneZoneEditorWidget::~SceneZoneEditorWidget()
{
    delete m_impl;
}

void SceneZoneEditorWidget::setName(const QString &t_value)
{
    m_impl->zone->setName(t_value);
}

void SceneZoneEditorWidget::setSize(const QVector3D &t_value)
{
    m_impl->zone->setSize(t_value);
}

void SceneZoneEditorWidget::chooseColor()
{
    const QColor c = QColorDialog::getColor(m_impl->zone->color(), this, "Zone Color");
    if (!c.isValid())
        return;
    m_impl->zone->setColor(c);
    m_impl->colorButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
}

void SceneZoneEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->zone->setPosition(t_value);
}

void SceneZoneEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->zone->setRotation(t_value);
}

void SceneZoneEditorWidget::refreshTransform()
{
    QSignalBlocker pb(m_impl->positionEdit);
    QSignalBlocker rb(m_impl->rotationEdit);
    m_impl->positionEdit->setValue(m_impl->zone->position());
    m_impl->rotationEdit->setValue(m_impl->zone->rotation());
}

void SceneZoneEditorWidget::refreshSize()
{
    QSignalBlocker sb(m_impl->sizeEdit);   // avoid feeding setSize back into a loop
    m_impl->sizeEdit->setValue(m_impl->zone->size());
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneZone::Impl
{
public:
    QVector3D size = QVector3D(4.0f, 4.0f, 4.0f);
    QColor color = QColor(80, 180, 255);
};

SceneZone::SceneZone() : SceneObject("zone"), m_impl(new Impl)
{
}

SceneZone::~SceneZone()
{
    delete m_impl;
}

QWidget *SceneZone::createEditor()
{
    return new SceneZoneEditorWidget(this);
}

void SceneZone::setSize(const QVector3D &t_value)
{
    m_impl->size = t_value;
    emit metadataChanged(this);
}

void SceneZone::setColor(const QColor &t_value)
{
    m_impl->color = t_value;
    emit metadataChanged(this);
}

QVector3D SceneZone::size() const
{
    return m_impl->size;
}

QColor SceneZone::color() const
{
    return m_impl->color;
}

bool SceneZone::containsPoint(const QVector3D &worldPoint) const
{
    const QVector3D local = globalMatrix().inverted().map(worldPoint);
    const QVector3D half = m_impl->size * 0.5f;
    return std::abs(local.x()) <= half.x()
        && std::abs(local.y()) <= half.y()
        && std::abs(local.z()) <= half.z();
}

static SceneZone *findZoneRecursive(SceneObject *obj, const QString &name)
{
    if (!obj)
        return nullptr;
    for (SceneObject *child : obj->sceneChildren())
    {
        if (child->typeId() == "zone" && child->name().compare(name, Qt::CaseInsensitive) == 0)
            return static_cast<SceneZone *>(child);
        if (SceneZone *found = findZoneRecursive(child, name))
            return found;
    }
    return nullptr;
}

SceneZone *SceneZone::findByName(Project *project, const QString &name)
{
    if (!project || name.trimmed().isEmpty())
        return nullptr;
    return findZoneRecursive(project->sceneRoot(), name.trimmed());
}

static void collectZoneNames(SceneObject *obj, QStringList &out)
{
    if (!obj)
        return;
    for (SceneObject *child : obj->sceneChildren())
    {
        if (child->typeId() == "zone")
            out << child->name();
        collectZoneNames(child, out);
    }
}

QStringList SceneZone::zoneNames(Project *project)
{
    QStringList names;
    if (project)
        collectZoneNames(project->sceneRoot(), names);
    return names;
}

void SceneZone::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneObject::readFromJson(t_json, t_context);
    if (t_json.contains("size"))
    {
        const QJsonObject s = t_json.value("size").toObject();
        m_impl->size = QVector3D(s.value("x").toDouble(m_impl->size.x()),
                                 s.value("y").toDouble(m_impl->size.y()),
                                 s.value("z").toDouble(m_impl->size.z()));
    }
    if (t_json.contains("color"))
        m_impl->color = QColor(t_json.value("color").toString());
}

void SceneZone::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);
    QJsonObject s;
    s.insert("x", m_impl->size.x());
    s.insert("y", m_impl->size.y());
    s.insert("z", m_impl->size.z());
    t_json.insert("size", s);
    t_json.insert("color", m_impl->color.name());
}

} // namespace photon
