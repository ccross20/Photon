#ifndef PHOTON_MASTERLAYERCHANNELEFFECT_H
#define PHOTON_MASTERLAYERCHANNELEFFECT_H

#include <QComboBox>
#include "sequence/channeleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class MasterLayerChannelEffect;

class MasterLayerEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    MasterLayerEffectEditor(MasterLayerChannelEffect *);

private slots:
    void comboChanged(int);

protected:
    void relayout(const QRectF &) override;

private:
    QComboBox *m_combo;
    MasterLayerChannelEffect *m_effect;
    RectangleGizmo *m_originHandle;
    QGraphicsPathItem *m_pathItem;

};


class MasterLayerChannelEffect : public ChannelEffect
{
public:
    MasterLayerChannelEffect();

    void setLayer(MasterLayer *);
    MasterLayer *layer() const{return m_layer;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void restore(Project &) override;
    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();


private:
    MasterLayer *m_layer = nullptr;
    QUuid m_layerGuid;
    double m_value = 1.0;
};

} // namespace photon


#endif // PHOTON_MASTERLAYERCHANNELEFFECT_H
