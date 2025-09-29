#ifndef FIXTUREGIZMO_H
#define FIXTUREGIZMO_H

#include <QObject>
#include "surfacegizmo.h"
#include "state/state.h"

namespace photon {

class FixtureGizmo : public SurfaceGizmo
{
    Q_OBJECT
public:
    FixtureGizmo(QObject *t_parent = nullptr);
    ~FixtureGizmo();

    void addMaskEffect(MaskEffect *);
    void removeMaskEffect(MaskEffect *);
    MaskEffect *maskEffectAtIndex(int index) const;
    int maskEffectCount() const;
    const QVector<Fixture*> maskedFixtures() const;

    void addFalloffEffect(FalloffEffect *);
    void removeFalloffEffect(FalloffEffect *);
    FalloffEffect *falloffEffectAtIndex(int index) const;
    int falloffEffectCount() const;
    void setDefaultFalloff(double);
    double defaultFalloff() const;
    double falloff(Fixture *t_fixture) const;

    State *state() const;

    virtual void processChannels(ProcessContext &) override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

signals:

    void falloffEffectAdded(photon::FalloffEffect*);
    void falloffEffectRemoved(photon::FalloffEffect*);
    void maskAdded(photon::MaskEffect *);
    void maskRemoved(photon::MaskEffect *);
    void maskUpdated(photon::MaskEffect *);
    void maskMoved(photon::MaskEffect *);
    void falloffMapChanged(photon::FixtureFalloffMap *);
    void falloffUpdated(photon::FalloffEffect *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photo

#endif // FIXTUREGIZMO_H
