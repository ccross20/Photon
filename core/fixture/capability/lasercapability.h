#ifndef PHOTON_LASERCAPABILITY_H
#define PHOTON_LASERCAPABILITY_H

#include "fixturecapability.h"

namespace photon {

// One channel of a DMX-controlled laser (e.g. Pangolin FB4), declared in a
// fixture definition as {"type": "Laser", "function": "<name>"}. Values are
// written as exact 8- or 16-bit DMX words rather than through
// DMXMatrix::setValuePercent, because lasers treat specific words as
// commands (32768 = centered / "keep angle, don't rotate", 251 = output on).
class PHOTONCORE_EXPORT LaserCapability : public FixtureCapability
{
public:
    enum Function
    {
        Function_Unknown,
        Function_Mode,
        Function_SetupIntensity,
        Function_SetupTestFrame,
        Function_SetupSizeX,
        Function_SetupSizeY,
        Function_SetupPositionX,
        Function_SetupPositionY,
        Function_SetupRotation,
        Function_Page,
        Function_Cue,
        Function_CueSpeed,
        Function_Zoom,
        Function_SizeX,
        Function_SizeY,
        Function_Angle,
        Function_Rotation,
        Function_PositionX,
        Function_PositionY,
        Function_ScanRate,
        Function_Red,
        Function_Green,
        Function_Blue,
        Function_ColorMix,
        Function_VisibleStart,
        Function_VisibleEnd,
        Function_Strobe
    };

    static constexpr int ModeOff = 0;
    static constexpr int ModeSetup = 240;
    static constexpr int ModeOutput = 251;

    LaserCapability();

    Function function() const;
    bool is16Bit() const;

    // Raw DMX word: 0-255, or 0-65535 when the channel has a fine channel.
    void setRaw(int value, DMXMatrix &matrix, double blend = 1.0) const;
    int raw(const DMXMatrix &matrix) const;

    // -1..1 mapped onto a centered channel (0 = -100%, 32768 = 0, 65535 = +100%).
    void setCentered(double value, DMXMatrix &matrix, double blend = 1.0) const;
    // 0..1 mapped across the channel's full range.
    void setPercent(double value, DMXMatrix &matrix, double blend = 1.0) const;

    // The word written before any state runs, so a channel nobody drives sits
    // at its "no change" value instead of 0 (which is -100% on centered
    // channels and 0% cue speed).
    int neutralRaw() const;

    void readFromOpenFixtureJson(const QJsonObject &) override;

    static LaserCapability *find(const Fixture *, Function);
    static void writeNeutralValues(const Fixture *, DMXMatrix &);

private:
    Function m_function = Function_Unknown;
};

} // namespace photon

#endif // PHOTON_LASERCAPABILITY_H
