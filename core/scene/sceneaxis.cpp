#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "sceneaxis.h"
#include "gui/vector3edit.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneAxisEditorWidget::Impl
{
public:
    Impl();
    SceneAxis *axis = nullptr;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QDoubleSpinBox *sizeSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneAxisEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return axis ? axis->tags() : QStringList(); },
        [this](const QStringList &tags){ if(axis) axis->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    formLayout->addRow("Tags", tagEditor);

    sizeSpin = new QDoubleSpinBox;
    sizeSpin->setMinimum(.1);
    sizeSpin->setMaximum(50);
    formLayout->addRow("Size", sizeSpin);
}

SceneAxisEditorWidget::SceneAxisEditorWidget(SceneAxis *t_axis, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    setLayout(m_impl->formLayout);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->axis = t_axis;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneAxisEditorWidget::setName);
    connect(m_impl->sizeSpin, &QDoubleSpinBox::valueChanged, this, &SceneAxisEditorWidget::setSize);

    connect(t_axis, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_axis->name());
    m_impl->tagEditor->refresh();
    m_impl->sizeSpin->setValue(t_axis->size());

    addHelperPropertyRows(m_impl->formLayout, t_axis, this);

    m_impl->positionEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Position", m_impl->positionEdit);
    m_impl->rotationEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Rotation", m_impl->rotationEdit);

    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneAxisEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneAxisEditorWidget::setRotation);

    m_impl->positionEdit->setValue(t_axis->position());
    m_impl->rotationEdit->setValue(t_axis->rotation());
}

SceneAxisEditorWidget::~SceneAxisEditorWidget()
{
    delete m_impl;
}

void SceneAxisEditorWidget::setName(const QString &t_value)
{
    m_impl->axis->setName(t_value);
}

void SceneAxisEditorWidget::setSize(float t_value)
{
    m_impl->axis->setSize(t_value);
}

void SceneAxisEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->axis->setPosition(t_value);
}

void SceneAxisEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->axis->setRotation(t_value);
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneAxis::Impl
{
public:
    float size = 3.0f;
};

SceneAxis::SceneAxis() : SceneHelperObject("axis"), m_impl(new Impl)
{
    setColor(QColor(200, 100, 220));
}

SceneAxis::~SceneAxis()
{
    delete m_impl;
}

QWidget *SceneAxis::createEditor()
{
    return new SceneAxisEditorWidget(this);
}

void SceneAxis::setSize(float t_value)
{
    m_impl->size = t_value;
    emit metadataChanged(this);
}

float SceneAxis::size() const
{
    return m_impl->size;
}

void SceneAxis::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneHelperObject::readFromJson(t_json, t_context);
    m_impl->size = t_json.value("size").toDouble(m_impl->size);
}

void SceneAxis::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("size", m_impl->size);
}

} // namespace photon
