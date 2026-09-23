#include <QUuid>
#include <QJsonObject>
#include <QVBoxLayout>
#include "gradientresource.h"
#include "gui/resourceeditorwidget.h"
#include "gui/color/gradientwidget.h"

namespace photon {

class GradientResource::Impl
{
public:
    QByteArray uniqueId;
    QString name;
    Gradient gradient;
};

GradientResource::GradientResource() : m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

GradientResource::~GradientResource()
{
    delete m_impl;
}

QByteArray GradientResource::uniqueId() const
{
    return m_impl->uniqueId;
}

QString GradientResource::name() const
{
    return m_impl->name;
}

void GradientResource::setName(const QString &t_name)
{
    if(m_impl->name == t_name)
        return;
    m_impl->name = t_name;
    notifyResourceChanged();
}

Gradient GradientResource::gradient() const
{
    return m_impl->gradient;
}

void GradientResource::setGradient(const Gradient &t_gradient)
{
    if(m_impl->gradient == t_gradient)
        return;
    m_impl->gradient = t_gradient;
    notifyResourceChanged();
}

QWidget *GradientResource::createResourceEditor()
{
    auto *container = new QWidget;
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new ResourceEditorWidget(this));

    auto *gradientWidget = new GradientWidget(m_impl->gradient);
    connect(gradientWidget, &GradientWidget::gradientChanged, this, [this](const Gradient &g){
        setGradient(g);
    });
    layout->addWidget(gradientWidget);
    layout->addStretch(1);

    return container;
}

void GradientResource::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    if(t_json.contains("uniqueId"))
        m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    if(t_json.contains("gradient"))
        m_impl->gradient.readFromJson(t_json.value("gradient").toObject());
    readResourceJson(t_json);
}

void GradientResource::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString{m_impl->uniqueId});
    QJsonObject gradientObj;
    m_impl->gradient.writeToJson(gradientObj);
    t_json.insert("gradient", gradientObj);
    writeResourceJson(t_json);
}

} // namespace photon
