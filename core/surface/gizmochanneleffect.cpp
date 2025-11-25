#include <QComboBox>
#include "gizmochanneleffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "sequence/channel.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/surfacegizmo.h"

namespace photon {



GizmoChannelEffectEditor::GizmoChannelEffectEditor(GizmoChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    m_combo = new QComboBox;

    QVector<SurfaceGizmo*> gizmos;

    if(t_effect->gizmoType().isEmpty())
        gizmos = t_effect->container()->gizmos();
    else
        gizmos = t_effect->container()->gizmosByType(t_effect->gizmoType());
    int counter = 0;
    int matchIndex = -1;
    for(auto gizmo : gizmos){
        m_combo->addItem(gizmo->id(),gizmo->uniqueId());

        if(t_effect->gizmoId() == gizmo->uniqueId())
            matchIndex = counter;
        counter++;
    }

    if(matchIndex >= 0)
        m_combo->setCurrentIndex(matchIndex);
    connect(m_combo, &QComboBox::currentIndexChanged, this, &GizmoChannelEffectEditor::gizmoIdIndexChanged);


}

GizmoChannelEffectEditor::~GizmoChannelEffectEditor(){}

StackedParameterWidget *GizmoChannelEffectEditor::createParameterWidget(GizmoChannelEffect *t_effect) const
{


    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(m_combo, "Gizmo");

    return paramWidget;
}

void GizmoChannelEffectEditor::gizmoIdIndexChanged(int t_value)
{
    m_effect->setGizmoId(m_combo->currentData().toByteArray());
}


GizmoChannelEffect::GizmoChannelEffect(const QByteArray &t_type) : ChannelEffect(),m_gizmoType(t_type)
{

}

GizmoChannelEffect::~GizmoChannelEffect()
{

}

QByteArray GizmoChannelEffect::gizmoType() const
{
    return m_gizmoType;
}

QByteArray GizmoChannelEffect::gizmoId() const
{
    return m_gizmoId;
}

void GizmoChannelEffect::setGizmoId(const QByteArray &t_value)
{
    m_gizmoId = t_value;
    m_gizmo = m_container->findGizmoWithId(t_value);
    updated();
}

SurfaceGizmoContainer *GizmoChannelEffect::container() const
{
    return m_container;
}

SurfaceGizmo *GizmoChannelEffect::gizmo() const
{
    return m_gizmo;
}

void GizmoChannelEffect::addedToChannel()
{
    qDebug() << "add to channel";
    if(!m_container){
        m_container = channel()->gizmoContainer();

        if(m_gizmoId.isEmpty())
        {
            auto gizmos = m_container->gizmosByType(gizmoType());
            if(!gizmos.isEmpty())
                m_gizmoId = gizmos.last()->uniqueId();
        }


        m_gizmo = m_container->findGizmoWithUniqueId(m_gizmoId);
        qDebug() << "Gizmo id " << m_gizmoId;
    }

}

void GizmoChannelEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_gizmoId = t_json.value("gizmoId").toString().toLatin1();
}

void GizmoChannelEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("gizmoId", QString(m_gizmoId));
}

} // namespace photon
