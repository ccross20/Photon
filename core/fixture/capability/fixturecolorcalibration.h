#ifndef PHOTON_FIXTURECOLORCALIBRATION_H
#define PHOTON_FIXTURECOLORCALIBRATION_H

#include <QMap>
#include <QString>
#include <QVector>
#include "fixturecapability.h"

class QColor;
class QJsonObject;

namespace photon {

// Short display/serialization name for a color-mixing CapabilityType (Red,
// Green, Blue, Amber, Lime, White, Cyan, Magenta, Yellow, Indigo, UV) - used
// both for the calibration JSON's channel keys and the calibration dialog's
// slider labels. Returns an empty string for any other capability type.
PHOTONCORE_EXPORT QString colorChannelTypeName(CapabilityType);
PHOTONCORE_EXPORT CapabilityType colorChannelTypeFromName(const QString &);

// One calibrated reference point: a named hue (or Warm/Cool White) and the
// raw per-LED-channel percentages (0..1) the user tuned on the real fixture
// to reproduce it.
struct PHOTONCORE_EXPORT ColorCalibrationEntry
{
    QString name;
    double hueDegrees = 0.0;                 // meaningless for the white entries
    QMap<CapabilityType, double> channelPercents;

    bool isValid() const { return !channelPercents.isEmpty(); }

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;
};

// A fixture type's full set of calibrated reference points. Attaches to a
// fixture definition (keyed by its definitionPath, see
// FixtureColorCalibrationStore below) rather than to any one patched
// instance, so every fixture sharing that definition resolves colors the
// same way.
struct PHOTONCORE_EXPORT FixtureColorCalibration
{
    QVector<ColorCalibrationEntry> hues;   // chromatic anchors, sorted by hueDegrees
    ColorCalibrationEntry coolWhite;
    ColorCalibrationEntry warmWhite;

    // False until at least one chromatic hue has been calibrated - callers
    // fall back to the uncalibrated heuristic conversion while this is false.
    bool isValid() const;

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;
};

// Resolves an arbitrary color to raw channel percentages by interpolating
// between the two calibrated hues bracketing its hue angle, blending toward
// coolWhite as saturation drops, and scaling the result by value (brightness).
//
// A plain QColor carries no color-temperature information, so there is no
// principled way to choose warmWhite vs. coolWhite from an RGB input alone -
// desaturated input currently always resolves toward coolWhite. warmWhite is
// still captured and stored so it is available once/if a CCT-aware color
// input exists.
PHOTONCORE_EXPORT QMap<CapabilityType, double> resolveCalibratedPercents(const FixtureColorCalibration &, const QColor &);

// Persists calibration as a sidecar JSON file, keyed by the fixture
// definition's path, under the app's writable data directory (there is no
// writer for the read-only OpenFixture definition JSON itself).
namespace FixtureColorCalibrationStore {
    PHOTONCORE_EXPORT FixtureColorCalibration load(const QString &definitionPath);
    PHOTONCORE_EXPORT void save(const QString &definitionPath, const FixtureColorCalibration &);
}

} // namespace photon

#endif // PHOTON_FIXTURECOLORCALIBRATION_H
