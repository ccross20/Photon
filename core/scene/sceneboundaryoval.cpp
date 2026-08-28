#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include "sceneboundaryoval.h"
#include "gui/vector3edit.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneBoundaryOvalEditorWidget::Impl
{
public:
    Impl();
    SceneBoundaryOval *oval = nullptr;
    QFormLayout *formLayout;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QDoubleSpinBox *widthSpin;
    QDoubleSpinBox *heightSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneBoundaryOvalEditorWidget::Impl::Impl()
{
    formLayout = new QFormLayout;

    nameEdit = new QLineEdit;
    formLayout->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return oval ? oval->tags() : QStringList(); },
        [this](const QStringList &tags){ if(oval) oval->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    formLayout->addRow("Tags", tagEditor);

    widthSpin = new QDoubleSpinBox;
    widthSpin->setMinimum(0.1);
    widthSpin->setMaximum(200.0);
    formLayout->addRow("Width", widthSpin);

    heightSpin = new QDoubleSpinBox;
    heightSpin->setMinimum(0.1);
    heightSpin->setMaximum(200.0);
    formLayout->addRow("Height", heightSpin);
}

SceneBoundaryOvalEditorWidget::SceneBoundaryOvalEditorWidget(SceneBoundaryOval *t_oval, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    setLayout(m_impl->formLayout);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->oval = t_oval;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneBoundaryOvalEditorWidget::setName);
    connect(m_impl->widthSpin, &QDoubleSpinBox::valueChanged, this, &SceneBoundaryOvalEditorWidget::setWidth);
    connect(m_impl->heightSpin, &QDoubleSpinBox::valueChanged, this, &SceneBoundaryOvalEditorWidget::setHeight);

    connect(t_oval, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_oval->name());
    m_impl->tagEditor->refresh();
    m_impl->widthSpin->setValue(t_oval->width());
    m_impl->heightSpin->setValue(t_oval->height());

    addHelperPropertyRows(m_impl->formLayout, t_oval, this);

    m_impl->positionEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Position", m_impl->positionEdit);
    m_impl->rotationEdit = new Vector3Edit;
    m_impl->formLayout->addRow("Rotation", m_impl->rotationEdit);

    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneBoundaryOvalEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneBoundaryOvalEditorWidget::setRotation);

    m_impl->positionEdit->setValue(t_oval->position());
    m_impl->rotationEdit->setValue(t_oval->rotation());
}

SceneBoundaryOvalEditorWidget::~SceneBoundaryOvalEditorWidget()
{
    delete m_impl;
}

void SceneBoundaryOvalEditorWidget::setName(const QString &t_value)
{
    m_impl->oval->setName(t_value);
}

void SceneBoundaryOvalEditorWidget::setWidth(double t_value)
{
    m_impl->oval->setWidth(float(t_value));
}

void SceneBoundaryOvalEditorWidget::setHeight(double t_value)
{
    m_impl->oval->setHeight(float(t_value));
}

void SceneBoundaryOvalEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->oval->setPosition(t_value);
}

void SceneBoundaryOvalEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->oval->setRotation(t_value);
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneBoundaryOval::Impl
{
public:
    float width = 6.0f;
    float height = 4.0f;
};

SceneBoundaryOval::SceneBoundaryOval() : SceneHelperObject("boundaryoval"), m_impl(new Impl)
{
    setColor(QColor(220, 140, 200));
}

SceneBoundaryOval::~SceneBoundaryOval()
{
    delete m_impl;
}

QWidget *SceneBoundaryOval::createEditor()
{
    return new SceneBoundaryOvalEditorWidget(this);
}

void SceneBoundaryOval::setWidth(float t_value)
{
    m_impl->width = t_value;
    emit metadataChanged(this);
}

void SceneBoundaryOval::setHeight(float t_value)
{
    m_impl->height = t_value;
    emit metadataChanged(this);
}

float SceneBoundaryOval::width() const
{
    return m_impl->width;
}

float SceneBoundaryOval::height() const
{
    return m_impl->height;
}

void SceneBoundaryOval::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneHelperObject::readFromJson(t_json, t_context);
    m_impl->width = t_json.value("width").toDouble(m_impl->width);
    m_impl->height = t_json.value("height").toDouble(m_impl->height);
}

void SceneBoundaryOval::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("width", m_impl->width);
    t_json.insert("height", m_impl->height);
}

} // namespace photon
