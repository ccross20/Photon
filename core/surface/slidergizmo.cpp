#include "slidergizmo.h"
#include "viewer/slidergizmowidget.h"

namespace photon {

const QByteArray SliderGizmo::GizmoId = "Slider";

SliderGizmo::SliderGizmo():SurfaceGizmo("Slider") {}

QString SliderGizmo::text() const
{
    return m_text;
}
void SliderGizmo::setText(const QString &t_text)
{
    m_text = t_text;
    markChanged();
}

double SliderGizmo::minValue() const
{
    return m_min;
}

void SliderGizmo::setMinValue(double t_value)
{
    m_min = t_value;
    markChanged();
}

double SliderGizmo::maxValue() const
{
    return m_max;
}

void SliderGizmo::setMaxValue(double t_value)
{
    m_max = t_value;
    markChanged();
}

double SliderGizmo::value() const
{
    return m_value;
}

void SliderGizmo::setValue(double t_value)
{
    m_value = t_value;
    markChanged();
}

SurfaceGizmoWidget *SliderGizmo::createWidget(SurfaceMode mode)
{
    auto widget = new SliderGizmoWidget(this, mode);
    connect(this, &SurfaceGizmo::gizmoUpdated, widget, &SurfaceGizmoWidget::updateGizmo);
    return widget;
}

void SliderGizmo::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceGizmo::readFromJson(t_json, t_context);

    m_text = t_json.value("text").toString(m_text);
    m_min = t_json.value("min").toDouble(m_min);
    m_max = t_json.value("max").toDouble(m_max);
    m_value = t_json.value("value").toDouble(m_value);
}

void SliderGizmo::writeToJson(QJsonObject &t_json) const
{
    SurfaceGizmo::writeToJson(t_json);
    t_json.insert("min", m_min);
    t_json.insert("max", m_max);
    t_json.insert("text", m_text);
    t_json.insert("value", m_value);
}

} // namespace photon
