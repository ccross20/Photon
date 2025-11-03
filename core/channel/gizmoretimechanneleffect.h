#ifndef GIZMORETIMECHANNELEFFECT_H
#define GIZMORETIMECHANNELEFFECT_H

#include "surface/gizmochanneleffect.h"

namespace photon {

class GizmoRetimeChannelEffect;

class GizmoRetimeChannelEffectEditor : public GizmoChannelEffectEditor
{
    Q_OBJECT
public:
    GizmoRetimeChannelEffectEditor(GizmoRetimeChannelEffect *);

private slots:


private:
    GizmoRetimeChannelEffect *m_effect;

};


class GizmoRetimeChannelEffect : public GizmoChannelEffect
{
public:
    GizmoRetimeChannelEffect();


    float * process(float *value, uint size, double time) const override;
    double processTime(double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

protected:
    void addedToChannel() override;


private:
    SurfaceGizmoContainer *m_container = nullptr;
};

} // namespace photon


#endif // GIZMORETIMECHANNELEFFECT_H
