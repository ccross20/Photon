#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "scenearrow.h"
#include "gui/vector3edit.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {



class SceneArrowEditorWidget::Impl
{
public:
    Impl();
    SceneArrow *arrow = nullptr;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QFormLayout *formLayout;
    QDoubleSpinBox *sizeSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneArrowEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return arrow ? arrow->tags() : QStringList(); },
        [this](const QStringList &tags){ if(arrow) arrow->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    formLayout->addRow("Tags", tagEditor);

    sizeSpin = new QDoubleSpinBox;
    sizeSpin->setMinimum(.1);
    sizeSpin->setMaximum(20);
    formLayout->addRow("Size", sizeSpin);
}

SceneArrowEditorWidget::SceneArrowEditorWidget(SceneArrow *t_arrow, QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    setLayout(m_impl->formLayout);

    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneArrowEditorWidget::setName);
    connect(m_impl->sizeSpin, &QDoubleSpinBox::valueChanged, this, &SceneArrowEditorWidget::setSize);

    m_impl->arrow = t_arrow;
    // Keeps the tag row live if a tag is added/removed from outside this
    // editor - e.g. dropped onto this object's row in the Project panel.
    connect(t_arrow, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_arrow->name());
    m_impl->tagEditor->refresh();
    m_impl->sizeSpin->setValue(t_arrow->size());

    addHelperPropertyRows(m_impl->formLayout, t_arrow, this);

    m_impl->positionEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Position", m_impl->positionEdit);
    m_impl->rotationEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Rotation", m_impl->rotationEdit);

    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneArrowEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneArrowEditorWidget::setRotation);

    m_impl->positionEdit->setValue(t_arrow->position());
    m_impl->rotationEdit->setValue(t_arrow->rotation());
}

SceneArrowEditorWidget::~SceneArrowEditorWidget()
{
    delete m_impl;
}

void SceneArrowEditorWidget::setName(const QString &t_value)
{
    m_impl->arrow->setName(t_value);
}

void SceneArrowEditorWidget::setSize(float t_value)
{
    m_impl->arrow->setSize(t_value);
}

void SceneArrowEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->arrow->setPosition(t_value);
}

void SceneArrowEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->arrow->setRotation(t_value);
}


class SceneArrow::Impl
{
public:
    float size = 1.0f;
};

SceneArrow::SceneArrow() : SceneHelperObject("arrow"),m_impl(new Impl)
{

}

SceneArrow::~SceneArrow()
{
    delete m_impl;
}

QWidget *SceneArrow::createEditor()
{
    return new SceneArrowEditorWidget(this);
}

void SceneArrow::setSize(float t_value)
{
    m_impl->size = t_value;
    emit metadataChanged(this);
}

float SceneArrow::size() const
{
    return m_impl->size;
}


void SceneArrow::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneHelperObject::readFromJson(t_json, t_context);

    m_impl->size = t_json.value("size").toDouble(m_impl->size);
}

void SceneArrow::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("size", m_impl->size);
}

} // namespace photon
