#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QSignalBlocker>
#include <QJsonObject>
#include <algorithm>
#include "scenebox.h"
#include "vector3edit.h"
#include "propertywidgets.h"
#include "tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneBoxEditorWidget::Impl
{
public:
    Impl();
    SceneBox *box = nullptr;
    PropertyForm *form;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    Vector3Edit *sizeEdit;
    QPushButton *colorButton;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

SceneBoxEditorWidget::Impl::Impl()
{
    form = new PropertyForm;

    form->addSection("General");

    nameEdit = new QLineEdit;
    form->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return box ? box->tags() : QStringList(); },
        [this](const QStringList &tags){ if(box) box->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    form->addRow("Tags", tagEditor);

    sizeEdit = new Vector3Edit(Vector3Edit::Distance);
    form->addRow("Size", sizeEdit);

    form->addSection("Style");

    colorButton = new QPushButton;
    form->addRow("Color", colorButton);

    form->addSection("Transform");

    positionEdit = new Vector3Edit(Vector3Edit::Distance);
    form->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit(Vector3Edit::Angle);
    form->addRow("Rotation", rotationEdit);
}

SceneBoxEditorWidget::SceneBoxEditorWidget(SceneBox *t_box, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_impl->form);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->box = t_box;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneBoxEditorWidget::setName);
    connect(m_impl->sizeEdit, &Vector3Edit::valueChanged, this, &SceneBoxEditorWidget::setSize);
    connect(m_impl->colorButton, &QPushButton::clicked, this, &SceneBoxEditorWidget::chooseColor);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneBoxEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneBoxEditorWidget::setRotation);

    connect(t_box, &SceneObject::positionChanged, this, &SceneBoxEditorWidget::refreshTransform);
    connect(t_box, &SceneObject::rotationChanged, this, &SceneBoxEditorWidget::refreshTransform);
    // Size changes (e.g. the scale gizmo) emit metadataChanged - keep the
    // field live, along with the tag row (tags dropped in the Project panel).
    connect(t_box, &SceneObject::metadataChanged, this, &SceneBoxEditorWidget::refreshSize);
    connect(t_box, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_box->name());
    m_impl->tagEditor->refresh();
    m_impl->sizeEdit->setValue(t_box->size());
    m_impl->positionEdit->setValue(t_box->position());
    m_impl->rotationEdit->setValue(t_box->rotation());

    const QColor c = t_box->color();
    m_impl->colorButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
}

SceneBoxEditorWidget::~SceneBoxEditorWidget()
{
    delete m_impl;
}

void SceneBoxEditorWidget::setName(const QString &t_value)
{
    m_impl->box->setName(t_value);
}

void SceneBoxEditorWidget::setSize(const QVector3D &t_value)
{
    m_impl->box->setSize(t_value);
}

void SceneBoxEditorWidget::chooseColor()
{
    const QColor c = QColorDialog::getColor(m_impl->box->color(), this, "Box Color");
    if (!c.isValid())
        return;
    m_impl->box->setColor(c);
    m_impl->colorButton->setStyleSheet(QString("background-color: %1;").arg(c.name()));
}

void SceneBoxEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->box->setPosition(t_value);
}

void SceneBoxEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->box->setRotation(t_value);
}

void SceneBoxEditorWidget::refreshTransform()
{
    QSignalBlocker pb(m_impl->positionEdit);
    QSignalBlocker rb(m_impl->rotationEdit);
    m_impl->positionEdit->setValue(m_impl->box->position());
    m_impl->rotationEdit->setValue(m_impl->box->rotation());
}

void SceneBoxEditorWidget::refreshSize()
{
    QSignalBlocker sb(m_impl->sizeEdit);
    m_impl->sizeEdit->setValue(m_impl->box->size());
}

// ─────────────────────────────────────────────────────────────────────────────

class SceneBox::Impl
{
public:
    QVector3D size = QVector3D(1.0f, 1.0f, 1.0f);
    QColor color = QColor(120, 120, 125);
};

SceneBox::SceneBox() : SceneObject("box"), m_impl(new Impl)
{
}

SceneBox::~SceneBox()
{
    delete m_impl;
}

QWidget *SceneBox::createEditor()
{
    return new SceneBoxEditorWidget(this);
}

void SceneBox::setSize(const QVector3D &t_value)
{
    const QVector3D size(std::max(t_value.x(), MinimumSide),
                         std::max(t_value.y(), MinimumSide),
                         std::max(t_value.z(), MinimumSide));
    if (m_impl->size == size)
        return;
    m_impl->size = size;
    emit metadataChanged(this);
}

void SceneBox::setColor(const QColor &t_value)
{
    m_impl->color = t_value;
    emit metadataChanged(this);
}

QVector3D SceneBox::size() const
{
    return m_impl->size;
}

QColor SceneBox::color() const
{
    return m_impl->color;
}

void SceneBox::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneObject::readFromJson(t_json, t_context);
    if (t_json.contains("size"))
    {
        const QJsonObject s = t_json.value("size").toObject();
        setSize(QVector3D(s.value("x").toDouble(m_impl->size.x()),
                          s.value("y").toDouble(m_impl->size.y()),
                          s.value("z").toDouble(m_impl->size.z())));
    }
    if (t_json.contains("color"))
        m_impl->color = QColor(t_json.value("color").toString());
}

void SceneBox::writeToJson(QJsonObject &t_json) const
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
