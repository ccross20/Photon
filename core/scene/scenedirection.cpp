#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "scenedirection.h"
#include "gui/vector3edit.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneDirectionEditorWidget::Impl
{
public:
    Impl();
    SceneDirection *direction = nullptr;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QDoubleSpinBox *sizeSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneDirectionEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return direction ? direction->tags() : QStringList(); },
        [this](const QStringList &tags){ if(direction) direction->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    formLayout->addRow("Tags", tagEditor);

    sizeSpin = new QDoubleSpinBox;
    sizeSpin->setMinimum(.1);
    sizeSpin->setMaximum(20);
    formLayout->addRow("Size", sizeSpin);
}

SceneDirectionEditorWidget::SceneDirectionEditorWidget(SceneDirection *t_direction, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    setLayout(m_impl->formLayout);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->direction = t_direction;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneDirectionEditorWidget::setName);
    connect(m_impl->sizeSpin, &QDoubleSpinBox::valueChanged, this, &SceneDirectionEditorWidget::setSize);

    connect(t_direction, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_direction->name());
    m_impl->tagEditor->refresh();
    m_impl->sizeSpin->setValue(t_direction->size());

    addHelperPropertyRows(m_impl->formLayout, t_direction, this);

    m_impl->positionEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Position", m_impl->positionEdit);
    m_impl->rotationEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Rotation", m_impl->rotationEdit);

    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneDirectionEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneDirectionEditorWidget::setRotation);

    m_impl->positionEdit->setValue(t_direction->position());
    m_impl->rotationEdit->setValue(t_direction->rotation());
}

SceneDirectionEditorWidget::~SceneDirectionEditorWidget()
{
    delete m_impl;
}

void SceneDirectionEditorWidget::setName(const QString &t_value)
{
    m_impl->direction->setName(t_value);
}

void SceneDirectionEditorWidget::setSize(float t_value)
{
    m_impl->direction->setSize(t_value);
}

void SceneDirectionEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->direction->setPosition(t_value);
}

void SceneDirectionEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->direction->setRotation(t_value);
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneDirection::Impl
{
public:
    float size = 1.5f;
};

SceneDirection::SceneDirection() : SceneHelperObject("direction"), m_impl(new Impl)
{
    setColor(QColor(80, 220, 140));
}

SceneDirection::~SceneDirection()
{
    delete m_impl;
}

QWidget *SceneDirection::createEditor()
{
    return new SceneDirectionEditorWidget(this);
}

void SceneDirection::setSize(float t_value)
{
    m_impl->size = t_value;
    emit metadataChanged(this);
}

float SceneDirection::size() const
{
    return m_impl->size;
}

void SceneDirection::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneHelperObject::readFromJson(t_json, t_context);
    m_impl->size = t_json.value("size").toDouble(m_impl->size);
}

void SceneDirection::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("size", m_impl->size);
}

} // namespace photon
