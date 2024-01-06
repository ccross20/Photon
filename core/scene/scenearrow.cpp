#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include "scenearrow.h"
#include "gui/vector3edit.h"

namespace photon {



class SceneArrowEditorWidget::Impl
{
public:
    Impl();
    SceneArrow *arrow;
    QLineEdit *nameEdit;
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



    sizeSpin = new QDoubleSpinBox;
    sizeSpin->setMinimum(.1);
    sizeSpin->setMaximum(20);
    formLayout->addRow("Size", sizeSpin);


    positionEdit = new Vector3Edit;
    formLayout->addRow("Position", positionEdit);

    rotationEdit = new Vector3Edit;
    formLayout->addRow("Rotation", rotationEdit);


}

SceneArrowEditorWidget::SceneArrowEditorWidget(SceneArrow *t_arrow, QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    setLayout(m_impl->formLayout);

    setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    connect(m_impl->nameEdit, &QLineEdit::textEdited, this, &SceneArrowEditorWidget::setName);
    connect(m_impl->sizeSpin, &QDoubleSpinBox::valueChanged, this, &SceneArrowEditorWidget::setSize);
    connect(m_impl->positionEdit, &Vector3Edit::valueChanged, this, &SceneArrowEditorWidget::setPosition);
    connect(m_impl->rotationEdit, &Vector3Edit::valueChanged, this, &SceneArrowEditorWidget::setRotation);

    m_impl->arrow = t_arrow;

    m_impl->nameEdit->setText(t_arrow->name());
    m_impl->sizeSpin->setValue(t_arrow->size());

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

SceneArrow::SceneArrow() : SceneObject("arrow"),m_impl(new Impl)
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
    SceneObject::readFromJson(t_json, t_context);

    m_impl->size = t_json.value("size").toDouble(m_impl->size);
}

void SceneArrow::writeToJson(QJsonObject &t_json) const
{
    SceneObject::writeToJson(t_json);
    t_json.insert("size", m_impl->size);
}

} // namespace photon
