#ifndef PHOTON_FIXTURECLIP_H
#define PHOTON_FIXTURECLIP_H
#include "clip.h"
#include "state/state.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureClip : public Clip
{
    Q_OBJECT
public:
    FixtureClip(QObject *parent = nullptr);
    FixtureClip(double start, double duration, QObject *parent = nullptr);
    virtual ~FixtureClip();


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
    virtual bool timeIsValid(double) const override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    static ClipInformation info();

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

#endif // PHOTON_FIXTURECLIP_H
