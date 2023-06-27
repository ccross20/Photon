#ifndef PHOTON_FIXTUREWHEEL_H
#define PHOTON_FIXTUREWHEEL_H
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureWheelSlot
{
public:

    enum SlotType{
        Slot_Color,
        Slot_Gobo,
        Slot_Open,
        Slot_Iris,
        Slot_AnimationGoboStart,
        Slot_AnimationGoboEnd
    };

    FixtureWheelSlot(SlotType, int);
    virtual ~FixtureWheelSlot();

    SlotType type() const;
    int index() const;
    virtual void readFromJson(const QJsonObject &);

private:
    class Impl;
    Impl *m_impl;
};


class PHOTONCORE_EXPORT ColorSlot : public FixtureWheelSlot
{
public:

    ColorSlot(int);
    ~ColorSlot();
    QColor color() const;
    QVector<QColor> colors() const;

    void readFromJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT GoboSlot : public FixtureWheelSlot
{
public:

    GoboSlot(int);
    ~GoboSlot();
    QImage image() const;
    void setImagePath(const QString &);
    QString imagePath() const;

    void readFromJson(const QJsonObject &) override;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT FixtureWheel
{
public:
    FixtureWheel(const QString &);
    ~FixtureWheel();

    QString name();
    FixtureWheelSlot *slot(int) const;
    int slotCount() const;
    const QVector<FixtureWheelSlot*> &allSlots() const;


    void readFromOpenFixtureJson(const QJsonObject &);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTUREWHEEL_H
