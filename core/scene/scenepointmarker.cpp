#include <QComboBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include "scenepointmarker.h"
#include "vector3edit.h"
#include "propertywidgets.h"
#include "tag/tageditorwidget.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

namespace {
// Index-parallel with the combo box items below.
const QStringList kShapeTokens = { "star", "cross", "circle", "square", "diamond" };
}

class ScenePointMarkerEditorWidget::Impl
{
public:
    Impl();
    ScenePointMarker *marker = nullptr;
    PropertyForm *form;
    QLineEdit *nameEdit;
    TagEditorWidget *tagEditor;
    QComboBox *shapeCombo;
    QDoubleSpinBox *sizeSpin;
    Vector3Edit *positionEdit;
    Vector3Edit *rotationEdit;
};

ScenePointMarkerEditorWidget::Impl::Impl()
{
    form = new PropertyForm;

    form->addSection("General");

    nameEdit = new QLineEdit;
    form->addRow("Name", nameEdit);

    tagEditor = new TagEditorWidget(
        [this](){ return marker ? marker->tags() : QStringList(); },
        [this](const QStringList &tags){ if(marker) marker->setTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    form->addRow("Tags", tagEditor);

    shapeCombo = new QComboBox;
    shapeCombo->addItems(QStringList() << "Star" << "Cross" << "Circle" << "Square" << "Diamond");
    form->addRow("Shape", shapeCombo);

    sizeSpin = new QDoubleSpinBox;
    sizeSpin->setSuffix(QStringLiteral(" m"));
    sizeSpin->setMinimum(.05);
    sizeSpin->setMaximum(20);
    form->addRow("Size", sizeSpin);
}

ScenePointMarkerEditorWidget::ScenePointMarkerEditorWidget(ScenePointMarker *t_marker, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_impl->form);
    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    m_impl->marker = t_marker;

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &ScenePointMarkerEditorWidget::setName);
    connect(m_impl->shapeCombo, &QComboBox::activated, this, &ScenePointMarkerEditorWidget::setShape);
    connect(m_impl->sizeSpin, &QDoubleSpinBox::valueChanged, this, &ScenePointMarkerEditorWidget::setSize);

    connect(t_marker, &SceneObject::metadataChanged, m_impl->tagEditor, &TagEditorWidget::refresh);

    m_impl->nameEdit->setText(t_marker->name());
    m_impl->tagEditor->refresh();
    m_impl->shapeCombo->setCurrentIndex(int(t_marker->shape()));
    m_impl->sizeSpin->setValue(t_marker->size());

    addHelperPropertyRows(m_impl->form, t_marker, this);

    m_impl->form->addSection("Transform");

    m_impl->positionEdit = new Vector3Edit(Vector3Edit::Distance);
    m_impl->form->addRow("Position", m_impl->positionEdit);
    m_impl->rotationEdit = new Vector3Edit(Vector3Edit::Angle);
    m_impl->form->addRow("Rotation", m_impl->rotationEdit);

    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &ScenePointMarkerEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &ScenePointMarkerEditorWidget::setRotation);
    connect(t_marker, &SceneObject::positionChanged, this, &ScenePointMarkerEditorWidget::refreshTransform);
    connect(t_marker, &SceneObject::rotationChanged, this, &ScenePointMarkerEditorWidget::refreshTransform);

    m_impl->positionEdit->setValue(t_marker->position());
    m_impl->rotationEdit->setValue(t_marker->rotation());
}

ScenePointMarkerEditorWidget::~ScenePointMarkerEditorWidget()
{
    delete m_impl;
}

void ScenePointMarkerEditorWidget::setName(const QString &t_value)
{
    m_impl->marker->setName(t_value);
}

void ScenePointMarkerEditorWidget::setShape(int t_index)
{
    m_impl->marker->setShape(ScenePointMarker::Shape(t_index));
}

void ScenePointMarkerEditorWidget::setSize(double t_value)
{
    m_impl->marker->setSize(float(t_value));
}

void ScenePointMarkerEditorWidget::setPosition(const QVector3D &t_value)
{
    m_impl->marker->setPosition(t_value);
}

void ScenePointMarkerEditorWidget::setRotation(const QVector3D &t_value)
{
    m_impl->marker->setRotation(t_value);
}

void ScenePointMarkerEditorWidget::refreshTransform()
{
    QSignalBlocker pb(m_impl->positionEdit);
    QSignalBlocker rb(m_impl->rotationEdit);
    m_impl->positionEdit->setValue(m_impl->marker->position());
    m_impl->rotationEdit->setValue(m_impl->marker->rotation());
}

// ─────────────────────────────────────────────────────────────────────────────

class ScenePointMarker::Impl
{
public:
    Shape shape = ShapeStar;
    float size = 0.5f;
};

ScenePointMarker::ScenePointMarker() : SceneHelperObject("pointmarker"), m_impl(new Impl)
{
    setColor(QColor(255, 210, 60));
}

ScenePointMarker::~ScenePointMarker()
{
    delete m_impl;
}

QWidget *ScenePointMarker::createEditor()
{
    return new ScenePointMarkerEditorWidget(this);
}

void ScenePointMarker::setShape(Shape t_value)
{
    m_impl->shape = t_value;
    emit metadataChanged(this);
}

ScenePointMarker::Shape ScenePointMarker::shape() const
{
    return m_impl->shape;
}

void ScenePointMarker::setSize(float t_value)
{
    m_impl->size = t_value;
    emit metadataChanged(this);
}

float ScenePointMarker::size() const
{
    return m_impl->size;
}

void ScenePointMarker::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SceneHelperObject::readFromJson(t_json, t_context);
    m_impl->size = t_json.value("size").toDouble(m_impl->size);
    const int index = kShapeTokens.indexOf(t_json.value("shape").toString());
    if(index >= 0)
        m_impl->shape = Shape(index);
}

void ScenePointMarker::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("size", m_impl->size);
    t_json.insert("shape", kShapeTokens.value(int(m_impl->shape), "star"));
}

} // namespace photon
