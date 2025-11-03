#ifndef FIXTUREACTION_H
#define FIXTUREACTION_H
#include "surfaceaction.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureAction : public SurfaceAction
{
    Q_OBJECT
public:
    FixtureAction(QObject *parent = nullptr);
    virtual ~FixtureAction();

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


public slots:
    void falloffUpdatedSlot(photon::FalloffEffect *);
    void maskUpdatedSlot(photon::MaskEffect *);

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

} // namespace photon

#endif // FIXTUREACTION_H
