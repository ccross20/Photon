#ifndef PHOTON_ABSTRACTFIXTURE_H
#define PHOTON_ABSTRACTFIXTURE_H
#include "photon-global.h"
#include "capability/fixturecapability.h"
#include "fixturechannel.h"
#include "scene/sceneobject.h"
#include "dmxreceiver.h"

namespace photon {

class ColorCapability;
class AngleCapability;

struct FixtureMode
{
    QString name;
    QString shortName;
    QStringList channels;

    bool operator==(const FixtureMode &other) const{return name == other.name;}
};


class PHOTONCORE_EXPORT Fixture : public SceneObject, public DMXReceiver
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


    // Values a laser only reads while in its setup profile - per-install
    // projection calibration, persisted with the fixture.
    struct LaserSetup
    {
        double masterIntensity = 1.0;   // 0..1
        int testFrame = 0;              // 0 = off, 1-255 = test_xxx animation
        double sizeX = 0.0;             // -1..1
        double sizeY = 0.0;
        double positionX = 0.0;
        double positionY = 0.0;
        double rotation = 0.0;          // degrees, 0..360

        bool operator==(const LaserSetup &) const = default;
    };

    Fixture(const QString &path = QString{});
    ~Fixture();

    bool isLaser() const;

    // Runtime only, never saved: a laser's output stays disabled (mode
    // channel held off by OutputOverridesNode) until it is explicitly armed.
    bool isLaserArmed() const;
    void setLaserArmed(bool);

    // Runtime only: while active, OutputOverridesNode holds the mode channel at
    // the setup value and writes laserSetup() onto the setup channels.
    bool isLaserSetupActive() const;
    void setLaserSetupActive(bool);

    LaserSetup laserSetup() const;
    void setLaserSetup(const LaserSetup &);

    // Folder of per-cue preview media for the visualizer, named after the
    // laser's own content layout: P001C003.mp4 / .gif / .png / ...
    QString laserPreviewFolder() const;
    void setLaserPreviewFolder(const QString &);

    QString description() const;
    QString manufacturer() const;
    QString comments() const;
    QString identifier() const;
    QStringList categories() const;

    // Visualiser model type override ("" = auto-detect from category).
    QString modelType() const;
    void setModelType(const QString &);

    // Visualiser beam style override: "" = Auto (follow the global toggle),
    // "cones" = basic cone, "volumetric" = raymarched volumetric beam,
    // "none" = no beam rendered at all.
    QString beamStyle() const;
    void setBeamStyle(const QString &);

    // Degrees added to the pan/tilt channel before it's written to DMX (see
    // AngleCapability::writePercent), so a fixture mounted at an odd angle -
    // rotated on its yoke, hung upside-down, whatever - can still be aimed as
    // if it were mounted the "normal" way: Pan/Tilt centered (0%) points
    // wherever it actually needs to for that fixture, without needing to
    // rewrite every cue/effect that aims it. Doesn't touch the fixture's own
    // Position/Rotation, which stays an accurate record of how it's really
    // mounted.
    float panOffset() const;
    void setPanOffset(float);
    float tiltOffset() const;
    void setTiltOffset(float);

    // Reverses the pan/tilt channel's direction of travel (mirrored about its
    // own center) before it's written to DMX (see AngleCapability::
    // writePercent) - for a fixture mounted flipped (upside-down, or rotated
    // 180 on its yoke) so "more" still moves it the way it's expected to,
    // rather than backwards. Applied before panOffset/tiltOffset, so the
    // offset is always measured in whichever direction the fixture now
    // actually responds to.
    bool panFlip() const;
    void setPanFlip(bool);
    bool tiltFlip() const;
    void setTiltFlip(bool);

    // Flips the visualiser's own preview of the pan/tilt direction of travel
    // - for eyeballing how a fixture mounted flipped would look without
    // needing to actually flip it. Visualiser-only: unlike panFlip/tiltFlip
    // above, this never touches the real DMX value (see
    // RhiRenderer::updateFixtureMotion).
    bool panInvert() const;
    void setPanInvert(bool);
    bool tiltInvert() const;
    void setTiltInvert(bool);

    void setComments(const QString &);
    void setIdentifier(const QString &);

    void setDMXOffset(int offset);
    int dmxOffset() const override;
    int dmxSize() const override;
    int universe() const override;
    int uniqueIndex() const;
    void setUniqueIndex(int);
    void setUniverse(int universe);

    int colorCount() const;
    ColorCapability *colorAtIndex(int) const;

    // Typed convenience accessors (null if the fixture lacks the capability). These
    // hide which lookup mechanism applies — e.g. the colour aggregate isn't stored
    // per-channel, and pan/tilt/zoom/focus are all AngleCapability instances.
    ColorCapability *color() const;
    AngleCapability *pan() const;
    AngleCapability *tilt() const;
    AngleCapability *zoom() const;
    AngleCapability *focus() const;

    const QVector<FixtureChannel*> &channels() const;
    FixtureChannel* findChannelWithName(const QString &name) const;

    const QVector<FixtureWheel*> &wheels() const;
    FixtureWheel *findWheel(const QString &) const;
    // Maps a generic wheel role (e.g. "color wheel", "gobo wheel 2") to this fixture's
    // actual wheel name, by classifying each wheel's slot types. Empty if none match.
    // Lets fixture nodes offer a fixed dropdown instead of per-fixture name typing.
    QString resolveWheelName(const QString &role) const;
    QVector<FixtureCapability*> findCapability(CapabilityType type, int index = 0) const;
    QVector<FixtureCapability*> findCapability(CapabilityType type, const QString &name) const;
    // Names of this fixture's channels that carry at least one capability of the
    // given type - e.g. for populating a "which channel" dropdown (the same name
    // findCapability(type, name) matches against), rather than requiring free-text.
    QStringList channelNamesForCapability(CapabilityType type) const;

    template<class V>
    QVector<V> findCapability() const
    {
        QVector<V> results;

        for(auto it = channels().cbegin(); it != channels().cend(); ++it)
        {
            auto channel = *it;

            // Skip channels outside the selected mode (see the non-template
            // findCapability overloads).
            if(!channel->isValid())
                continue;

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
    QString definitionPath() const;

    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

signals:
    void WillBeDestroyed();

private:

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ABSTRACTFIXTURE_H
