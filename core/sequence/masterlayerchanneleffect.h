#ifndef PHOTON_MASTERLAYERCHANNELEFFECT_H
#define PHOTON_MASTERLAYERCHANNELEFFECT_H

#include <QComboBox>
#include "sequence/channeleffect.h"

namespace photon {

class MasterLayerChannelEffect;

class MasterLayerEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    MasterLayerEffectEditor(MasterLayerChannelEffect *);

private slots:
    void comboChanged(int);

private:
    QComboBox *m_combo;
    MasterLayerChannelEffect *m_effect;

};


class MasterLayerChannelEffect : public ChannelEffect
{
public:
    MasterLayerChannelEffect();

    void setLayer(MasterLayer *);
    MasterLayer *layer() const{return m_layer;}
    float * process(float *value, uint size, double time) const override;
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
