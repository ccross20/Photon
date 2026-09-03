#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "scenelinearfalloff.h"
#include "gui/vector3edit.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneLinearFalloffEditorWidget::Impl
{
public:
    Impl();
    SceneLinearFalloff *falloff = nullptr;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QDoubleSpinBox *lengthSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneLinearFalloffEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return falloff ? falloff->tags() : QStringList(); },
        [this](const QStringList &tags){ if(falloff) falloff->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    formLayout->addRow("Tags", tagEditor);

    lengthSpin = new QDoubleSpinBox;
    lengthSpin->setMinimum(.1);
    lengthSpin->setMaximum(200);
    formLayout->addRow("Length", lengthSpin);
}

SceneLinearFalloffEditorWidget::SceneLinearFalloffEditorWidget(SceneLinearFalloff *t_falloff, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    setLayout(m_impl->formLayout);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->falloff = t_falloff;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneLinearFalloffEditorWidget::setName);
    connect(m_impl->lengthSpin, &QDoubleSpinBox::valueChanged, this, &SceneLinearFalloffEditorWidget::setLength);

    connect(t_falloff, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_falloff->name());
    m_impl->tagEditor->refresh();
    m_impl->lengthSpin->setValue(t_falloff->length());

    addHelperPropertyRows(m_impl->formLayout, t_falloff, this);

    m_impl->positionEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Position", m_impl->positionEdit);
    m_impl->rotationEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Rotation", m_impl->rotationEdit);

    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneLinearFalloffEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneLinearFalloffEditorWidget::setRotation);

    m_impl->positionEdit->setValue(t_falloff->position());
    m_impl->rotationEdit->setValue(t_falloff->rotation());
}

SceneLinearFalloffEditorWidget::~SceneLinearFalloffEditorWidget()
{
    delete m_impl;
}

void SceneLinearFalloffEditorWidget::setName(const QString &t_value)
{
    m_impl->falloff->setName(t_value);
}

void SceneLinearFalloffEditorWidget::setLength(double t_value)
{
    m_impl->falloff->setLength(t_value);
}

void SceneLinearFalloffEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->falloff->setPosition(t_value);
}

void SceneLinearFalloffEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->falloff->setRotation(t_value);
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneLinearFalloff::Impl
{
public:
    float length = 2.0f;
};

SceneLinearFalloff::SceneLinearFalloff() : SceneHelperObject("linearfalloff"), m_impl(new Impl)
{
    setColor(QColor(240, 180, 90));
}

SceneLinearFalloff::~SceneLinearFalloff()
{
    delete m_impl;
}

QWidget *SceneLinearFalloff::createEditor()
{
    return new SceneLinearFalloffEditorWidget(this);
}

void SceneLinearFalloff::setLength(float t_value)
{
    m_impl->length = t_value;
    emit metadataChanged(this);
}

float SceneLinearFalloff::length() const
{
    return m_impl->length;
}

void SceneLinearFalloff::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneHelperObject::readFromJson(t_json, t_context);
    m_impl->length = t_json.value("length").toDouble(m_impl->length);
}

void SceneLinearFalloff::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("length", m_impl->length);
}

} // namespace photon
