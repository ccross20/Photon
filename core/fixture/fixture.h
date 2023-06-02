#ifndef PHOTON_ABSTRACTFIXTURE_H
#define PHOTON_ABSTRACTFIXTURE_H
#include "photon-global.h"
#include "capability/fixturecapability.h"
#include "fixturechannel.h"
#include "scene/sceneobject.h"

namespace photon {

struct FixtureMode
{
    QString name;
    QString shortName;
    QStringList channels;

    bool operator==(const FixtureMode &other) const{return name == other.name;}
};


class PHOTONCORE_EXPORT Fixture : public SceneObject
{
    Q_OBJECT
public:

    const static QByteArray FixtureMime;

    enum FixtureType{
        Fixture_Unknown,
        Fixture_BarrelScanner,
        Fixture_Blinder,
        Fixture_ColorChanger,
        Fixture_Dimmer,
        Fixture_Effect,
        Fixture_Fan,
        Fixture_Flower,
        Fixture_Hazer,
        Fixture_Laser,
        Fixture_Matrix,
        Fixture_MovingHead,
        Fixture_PixelBar,
        Fixture_Scanner,
        Fixture_Smoke,
        Fixture_Stand,
        Fixture_Strobe,
        Fixture_Other
    };

    enum DMXConnector{
        Connector_Unknown,
        Connector_3Pin,
        Connector_3PinSwapped,
        Connector_3PinXLRIP65,
        Connector_5Pin,
        Connector_5PinXLRIP65,
        Connector_3PinAnd5Pin,
        Connector_3_5_StereoJack
    };


    struct Physical
    {
        QVector3D dimensions;
        float weight = 0.f;
        float power = 0.f;
        DMXConnector dmxConnector = Connector_Unknown;
        QString bulbDescription;
        float colorTemperature = 0.f;
        float lumens = 0.f;
        float lensMinimum = 1.0f;
        float lensMaximum = 25.0f;
        QVector3D matrixPixelDimensions;
        QVector3D matrixPixelSpacing;
    };


    Fixture(const QString &path = QString{});
    ~Fixture();

    QString description() const;
    QString manufacturer() const;
    QString comments() const;
    QString identifier() const;

    void setComments(const QString &);
    void setIdentifier(const QString &);

    void setDMXOffset(int offset);
    int dmxOffset() const;
    int dmxSize() const;
    int universe() const;
    int uniqueIndex() const;
    void setUniqueIndex(int);
    void setUniverse(int universe);

    const QVector<FixtureChannel*> &channels() const;
    FixtureChannel* findChannelWithName(const QString &name) const;

    QVector<FixtureCapability*> findCapability(CapabilityType type, int index = 0) const;

    template<class V>
    QVector<V> findCapability() const
    {
        QVector<V> results;

        for(auto it = channels().cbegin(); it != channels().cend(); ++it)
        {
            auto channel = *it;

            for(auto capabilityIt = channel->capabilities().cbegin(); capabilityIt != channel->capabilities().cend(); ++capabilityIt)
            {
                auto cap = dynamic_cast<V>(*capabilityIt);
                if(cap)
                    results.append(cap);
            }
        }

        return results;
    }


    QWidget *createEditor() override;

    QVector<FixtureMode> modes() const;

    void setMode(uchar mode);
    int mode() const;

    Physical physical() const;

    void loadFixtureDefinition(const QString &path);
    void readFromOpenFixtureJson(const QJsonObject &);

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

signals:
    void WillBeDestroyed();

private:

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ABSTRACTFIXTURE_H
