#include <QPen>
#include "masterlayerchanneleffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "masterlayer.h"
#include "sequence.h"
#include "channel.h"

namespace photon {

MasterLayerEffectEditor::MasterLayerEffectEditor(MasterLayerChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);


    m_combo = new QComboBox;
    m_combo->addItem("None","");

    int index = 0;
    auto sequence = m_effect->channel()->sequence();

    if(sequence)
    {
        for(auto layer : sequence->layers())
        {
            MasterLayer *master = dynamic_cast<MasterLayer*>(layer);
            if(master)
            {
                m_combo->addItem(master->name(), master->guid().toString());
                if(master == m_effect->layer())
                    index = m_combo->count() - 1;
            }
        }
    }

    m_combo->setCurrentIndex(index);
    connect(m_combo, &QComboBox::currentIndexChanged, this, &MasterLayerEffectEditor::comboChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(m_combo, "Master Layer");

    addWidget(paramWidget, "Constant");


}

void MasterLayerEffectEditor::comboChanged(int t_index)
{
    if(t_index == 0)
        m_effect->setLayer(nullptr);


    auto sequence = m_effect->channel()->sequence();

    if(sequence)
    {
        auto layer = dynamic_cast<MasterLayer*>(sequence->findLayerByGuid(QUuid::fromString(m_combo->itemData(t_index).toString())));

        if(layer)
            m_effect->setLayer(layer);
    }
}

void MasterLayerEffectEditor::relayout(const QRectF &t_sceneRect)
{

}



EffectInformation MasterLayerChannelEffect::info()
{
    EffectInformation toReturn([](){return new MasterLayerChannelEffect;});
    toReturn.name = "Master Layer";
    toReturn.effectId = "photon.effect.master";
    toReturn.categories.append("Generator");

    return toReturn;
}



MasterLayerChannelEffect::MasterLayerChannelEffect() : ChannelEffect()
{

}

void MasterLayerChannelEffect::setLayer(MasterLayer *t_layer)
{
    m_layer = t_layer;
    updated();

}

double MasterLayerChannelEffect::process(double value, double time) const
{
    if(m_layer)
        return m_layer->process(channel()->startTime() + time);
    return m_value;
}

ChannelEffectEditor *MasterLayerChannelEffect::createEditor()
{
    return new MasterLayerEffectEditor(this);
}

void MasterLayerChannelEffect::restore(Project &t_project)
{
    ChannelEffect::restore(t_project);

    auto seq = channel()->sequence();

    if(seq && !m_layerGuid.isNull()){
        m_layer = dynamic_cast<MasterLayer*>(seq->findLayerByGuid(m_layerGuid));
    }
}

void MasterLayerChannelEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("layerGuid"))
    {
        m_layerGuid = QUuid::fromString(t_json.value("layerGuid").toString());
    }
}

void MasterLayerChannelEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    if(m_layer)
        t_json.insert("layerGuid", m_layer->guid().toString());

}

} // namespace photon
