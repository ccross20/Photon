#ifndef PHOTON_STUTTEREFFECT_H
#define PHOTON_STUTTEREFFECT_H

#include "sequence/channeleffect.h"
#include "gui/gizmo/gizmogroup.h"

namespace photon {


class StutterEffect;
class GizmoGroup;
class GizmoHandle;

class StutterEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    StutterEffectEditor(StutterEffect *);

private slots:
    void gapChanged(double);
    void durationChanged(double);

protected:
    void relayout(const QRectF &) override;

private:
    StutterEffect *m_effect;
    double m_referenceTime = 0;
    GizmoGroup *m_gizmos;
    GizmoHandle *m_originHandle;
    GizmoHandle *m_gapHandle;
    GizmoHandle *m_durationHandle;
};

class StutterEffect : public ChannelEffect
{
public:
    StutterEffect();

    void setGap(double);
    void setDuration(double);
    double gap() const{return m_gap;}
    double duration() const{return m_duration;}
    float * process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();

private:
    double m_gap = 1.0;
    double m_duration = 1.0;
};

} // namespace photon

#endif // PHOTON_STUTTEREFFECT_H
