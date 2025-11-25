#ifndef GIZMOCHANNELEFFECT_H
#define GIZMOCHANNELEFFECT_H
#include "sequence/channeleffect.h"

class QComboBox;

namespace photon {

class GizmoChannelEffect;
class StackedParameterWidget;

class PHOTONCORE_EXPORT GizmoChannelEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    GizmoChannelEffectEditor(GizmoChannelEffect *);
    virtual ~GizmoChannelEffectEditor();

    StackedParameterWidget *createParameterWidget(GizmoChannelEffect *) const;

private slots:
    void gizmoIdIndexChanged(int);

private:
    GizmoChannelEffect *m_effect;
    QComboBox *m_combo;

};


class PHOTONCORE_EXPORT GizmoChannelEffect : public ChannelEffect
{
public:
    GizmoChannelEffect(const QByteArray &t_type);
    virtual ~GizmoChannelEffect();

    virtual void readFromJson(const QJsonObject &) override;
    virtual void writeToJson(QJsonObject &) const override;

    QByteArray gizmoId() const;
    void setGizmoId(const QByteArray &);
    QByteArray gizmoType() const;
    SurfaceGizmoContainer *container() const;
    SurfaceGizmo *gizmo() const;

protected:
    void addedToChannel() override;

private:
    SurfaceGizmoContainer *m_container = nullptr;
    SurfaceGizmo *m_gizmo = nullptr;
    QByteArray m_gizmoId;
    QByteArray m_gizmoType;
};

} // namespace photon

#endif // GIZMOCHANNELEFFECT_H
