#include <cmath>
#include <QCheckBox>
#include <QMatrix4x4>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include "scenefalloff.h"
#include "propertycombobox.h"
#include "propertywidgets.h"
#include "tag/tageditorwidget.h"
#include "vector3edit.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

class SceneFalloff::Impl
{
public:
    Shape shape = ShapeLinear;
    float length = 2.0f;
    float sweep = 360.0f;
    Wrap wrap = WrapHold;
    bool mirrorAcrossX = false;
    bool mirrorAcrossY = false;
};

SceneFalloff::SceneFalloff() : SceneHelperObject("falloff"), m_impl(new Impl)
{
    setColor(QColor(240, 180, 90));
}

SceneFalloff::~SceneFalloff()
{
    delete m_impl;
}

SceneFalloff::Shape SceneFalloff::shape() const { return m_impl->shape; }
float SceneFalloff::length() const { return m_impl->length; }
float SceneFalloff::sweep() const { return m_impl->sweep; }
SceneFalloff::Wrap SceneFalloff::wrap() const { return m_impl->wrap; }
bool SceneFalloff::mirrorAcrossX() const { return m_impl->mirrorAcrossX; }
bool SceneFalloff::mirrorAcrossY() const { return m_impl->mirrorAcrossY; }

void SceneFalloff::setShape(Shape t_value)
{
    if(m_impl->shape == t_value)
        return;
    m_impl->shape = t_value;
    emit metadataChanged(this);
}

void SceneFalloff::setLength(float t_value)
{
    t_value = std::max(0.01f, t_value);
    if(m_impl->length == t_value)
        return;
    m_impl->length = t_value;
    emit metadataChanged(this);
}

void SceneFalloff::setSweep(float t_value)
{
    t_value = std::clamp(t_value, 1.0f, 360.0f);
    if(m_impl->sweep == t_value)
        return;
    m_impl->sweep = t_value;
    emit metadataChanged(this);
}

void SceneFalloff::setWrap(Wrap t_value)
{
    if(m_impl->wrap == t_value)
        return;
    m_impl->wrap = t_value;
    emit metadataChanged(this);
}

void SceneFalloff::setMirrorAcrossX(bool t_value)
{
    if(m_impl->mirrorAcrossX == t_value)
        return;
    m_impl->mirrorAcrossX = t_value;
    emit metadataChanged(this);
}

void SceneFalloff::setMirrorAcrossY(bool t_value)
{
    if(m_impl->mirrorAcrossY == t_value)
        return;
    m_impl->mirrorAcrossY = t_value;
    emit metadataChanged(this);
}

double SceneFalloff::rawValueAtLocal(const QVector3D &t_local) const
{
    double x = t_local.x();
    double y = t_local.y();
    if(m_impl->mirrorAcrossX)
        y = std::abs(y);
    if(m_impl->mirrorAcrossY)
        x = std::abs(x);

    switch(m_impl->shape)
    {
    case ShapeRadial:
        return std::hypot(x, y) / m_impl->length;
    case ShapeConical:
    {
        // Clockwise from +Y toward +X when the plane is seen from +Z.
        double angle = std::atan2(x, y);
        if(angle < 0.0)
            angle += 2.0 * M_PI;
        return angle / (m_impl->sweep * M_PI / 180.0);
    }
    case ShapeLinear:
    default:
        return y / m_impl->length;
    }
}

double SceneFalloff::wrapped(double t_raw, Wrap t_wrap)
{
    switch(t_wrap)
    {
    case WrapRepeat:
        return t_raw - std::floor(t_raw);
    case WrapPingPong:
    {
        const double phase = t_raw - 2.0 * std::floor(t_raw / 2.0);   // 0..2
        return phase <= 1.0 ? phase : 2.0 - phase;
    }
    case WrapHold:
    default:
        return std::clamp(t_raw, 0.0, 1.0);
    }
}

double SceneFalloff::amountAtLocal(const QVector3D &t_local) const
{
    return wrapped(rawValueAtLocal(t_local), m_impl->wrap);
}

double SceneFalloff::rawValueAt(const QVector3D &t_world) const
{
    return rawValueAtLocal(globalMatrix().inverted().map(t_world));
}

double SceneFalloff::amountAt(const QVector3D &t_world) const
{
    return amountAtLocal(globalMatrix().inverted().map(t_world));
}

QWidget *SceneFalloff::createEditor()
{
    auto *editor = new QWidget;
    auto *outer = new QVBoxLayout(editor);
    outer->setContentsMargins(0, 0, 0, 0);
    auto *form = new PropertyForm;
    outer->addWidget(form);
    editor->setSizePolicy(QSizePolicy{QSizePolicy::MinimumExpanding, QSizePolicy::Maximum});

    form->addSection("General");
    auto *nameEdit = new QLineEdit(name());
    form->addRow("Name", nameEdit);
    auto *tagEditor = new TagEditorWidget(
        [this](){ return tags(); },
        [this](const QStringList &newTags){ setTags(newTags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });
    form->addRow("Tags", tagEditor);
    tagEditor->refresh();

    form->addSection("Falloff");
    auto *shapeCombo = new PropertyComboBox;
    shapeCombo->addItems({"Linear", "Radial", "Conical"});
    form->addRow("Shape", shapeCombo);

    auto *lengthSpin = new QDoubleSpinBox;
    lengthSpin->setSuffix(QStringLiteral(" m"));
    lengthSpin->setRange(0.01, 500.0);
    lengthSpin->setDecimals(2);
    lengthSpin->setToolTip("Linear: distance to the end. Radial: radius to the end.\n"
                           "Conical: size of the disc drawn in the viewport.");
    form->addRow("Length", lengthSpin);

    auto *sweepSpin = new QDoubleSpinBox;
    sweepSpin->setRange(1.0, 360.0);
    sweepSpin->setDecimals(1);
    sweepSpin->setSuffix(QStringLiteral("°"));
    form->addRow("Sweep", sweepSpin);

    auto *wrapCombo = new PropertyComboBox;
    wrapCombo->addItems({"Hold", "Repeat", "Ping-Pong"});
    wrapCombo->setToolTip("What happens past the end of the gradient");
    form->addRow("Beyond End", wrapCombo);

    auto *mirrorX = new QCheckBox;
    mirrorX->setToolTip("Reflect the pattern across the falloff's local X axis");
    form->addRow("Mirror Across X", mirrorX);
    auto *mirrorY = new QCheckBox;
    mirrorY->setToolTip("Reflect the pattern across the falloff's local Y axis");
    form->addRow("Mirror Across Y", mirrorY);

    addHelperPropertyRows(form, this, editor);

    form->addSection("Transform");
    auto *positionEdit = new Vector3Edit(Vector3Edit::Distance);
    form->addRow("Position", positionEdit);
    auto *rotationEdit = new Vector3Edit(Vector3Edit::Angle);
    form->addRow("Rotation", rotationEdit);

    // Pulls every field from the object; also re-run on outside changes (undo,
    // dragging the gizmo, another editor).
    auto refresh = [=]() {
        const QSignalBlocker blockers[] = {
            QSignalBlocker(shapeCombo), QSignalBlocker(lengthSpin), QSignalBlocker(sweepSpin),
            QSignalBlocker(wrapCombo), QSignalBlocker(mirrorX), QSignalBlocker(mirrorY),
            QSignalBlocker(positionEdit), QSignalBlocker(rotationEdit),
        };
        if(!nameEdit->hasFocus())
            nameEdit->setText(name());
        shapeCombo->setCurrentIndex(int(shape()));
        lengthSpin->setValue(length());
        sweepSpin->setValue(sweep());
        sweepSpin->setEnabled(shape() == ShapeConical);
        wrapCombo->setCurrentIndex(int(wrap()));
        mirrorX->setChecked(mirrorAcrossX());
        mirrorY->setChecked(mirrorAcrossY());
        positionEdit->setValue(position());
        rotationEdit->setValue(rotation());
    };
    refresh();

    connect(nameEdit, &QLineEdit::textEdited, this, &SceneFalloff::setName);
    connect(shapeCombo, &QComboBox::activated, this, [this](int index){ setShape(Shape(index)); });
    connect(lengthSpin, &QDoubleSpinBox::valueChanged, this, [this](double v){ setLength(float(v)); });
    connect(sweepSpin, &QDoubleSpinBox::valueChanged, this, [this](double v){ setSweep(float(v)); });
    connect(wrapCombo, &QComboBox::activated, this, [this](int index){ setWrap(Wrap(index)); });
    connect(mirrorX, &QCheckBox::toggled, this, &SceneFalloff::setMirrorAcrossX);
    connect(mirrorY, &QCheckBox::toggled, this, &SceneFalloff::setMirrorAcrossY);
    connect(positionEdit, &Vector3Edit::valueChanged, this, &SceneObject::setPosition);
    connect(rotationEdit, &Vector3Edit::valueChanged, this, &SceneObject::setRotation);

    connect(this, &SceneObject::metadataChanged, editor, [refresh, tagEditor](){ refresh(); tagEditor->refresh(); });
    connect(this, &SceneObject::positionChanged, editor, refresh);
    connect(this, &SceneObject::rotationChanged, editor, refresh);
    return editor;
}

void SceneFalloff::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    // The base class takes its type id from the file, which for a Linear
    // Falloff this replaced is "linearfalloff" - keep it "falloff" so it's
    // recognised (and re-saved) as what it now is.
    QJsonObject json = t_json;
    json.insert("typeId", QStringLiteral("falloff"));
    SceneHelperObject::readFromJson(json, t_context);
    // "length" is also what the Linear Falloff this replaced stored, so those
    // load as linear falloffs unchanged.
    m_impl->shape = Shape(std::clamp(t_json.value("shape").toInt(ShapeLinear), 0, int(ShapeConical)));
    m_impl->length = std::max(0.01f, float(t_json.value("length").toDouble(m_impl->length)));
    m_impl->sweep = std::clamp(float(t_json.value("sweep").toDouble(m_impl->sweep)), 1.0f, 360.0f);
    m_impl->wrap = Wrap(std::clamp(t_json.value("wrap").toInt(WrapHold), 0, int(WrapPingPong)));
    m_impl->mirrorAcrossX = t_json.value("mirrorAcrossX").toBool(false);
    m_impl->mirrorAcrossY = t_json.value("mirrorAcrossY").toBool(false);
}

void SceneFalloff::writeToJson(QJsonObject &t_json) const
{
    SceneHelperObject::writeToJson(t_json);
    t_json.insert("shape", int(m_impl->shape));
    t_json.insert("length", m_impl->length);
    t_json.insert("sweep", m_impl->sweep);
    t_json.insert("wrap", int(m_impl->wrap));
    t_json.insert("mirrorAcrossX", m_impl->mirrorAcrossX);
    t_json.insert("mirrorAcrossY", m_impl->mirrorAcrossY);
}

} // namespace photon
