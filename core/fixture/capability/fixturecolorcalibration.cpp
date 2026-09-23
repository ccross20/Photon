#include <algorithm>
#include <QColor>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "fixturecolorcalibration.h"
#include "photoncore.h"

namespace photon {

QString colorChannelTypeName(CapabilityType t_type)
{
    switch(t_type)
    {
    case Capability_Red:     return QStringLiteral("red");
    case Capability_Green:   return QStringLiteral("green");
    case Capability_Blue:    return QStringLiteral("blue");
    case Capability_White:   return QStringLiteral("white");
    case Capability_Amber:   return QStringLiteral("amber");
    case Capability_Lime:    return QStringLiteral("lime");
    case Capability_Cyan:    return QStringLiteral("cyan");
    case Capability_Magenta: return QStringLiteral("magenta");
    case Capability_Yellow:  return QStringLiteral("yellow");
    case Capability_Indigo:  return QStringLiteral("indigo");
    case Capability_UV:      return QStringLiteral("uv");
    default: return QString();
    }
}

CapabilityType colorChannelTypeFromName(const QString &t_name)
{
    if(t_name == QStringLiteral("red")) return Capability_Red;
    if(t_name == QStringLiteral("green")) return Capability_Green;
    if(t_name == QStringLiteral("blue")) return Capability_Blue;
    if(t_name == QStringLiteral("white")) return Capability_White;
    if(t_name == QStringLiteral("amber")) return Capability_Amber;
    if(t_name == QStringLiteral("lime")) return Capability_Lime;
    if(t_name == QStringLiteral("cyan")) return Capability_Cyan;
    if(t_name == QStringLiteral("magenta")) return Capability_Magenta;
    if(t_name == QStringLiteral("yellow")) return Capability_Yellow;
    if(t_name == QStringLiteral("indigo")) return Capability_Indigo;
    if(t_name == QStringLiteral("uv")) return Capability_UV;
    return Capability_Unknown;
}

void ColorCalibrationEntry::readFromJson(const QJsonObject &t_json)
{
    name = t_json.value("name").toString();
    hueDegrees = t_json.value("hue").toDouble(0.0);

    channelPercents.clear();
    const QJsonObject channels = t_json.value("channels").toObject();
    for(auto it = channels.constBegin(); it != channels.constEnd(); ++it)
    {
        const CapabilityType type = colorChannelTypeFromName(it.key());
        if(type != Capability_Unknown)
            channelPercents.insert(type, it.value().toDouble());
    }
}

void ColorCalibrationEntry::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", name);
    t_json.insert("hue", hueDegrees);

    QJsonObject channels;
    for(auto it = channelPercents.cbegin(); it != channelPercents.cend(); ++it)
    {
        const QString key = colorChannelTypeName(it.key());
        if(!key.isEmpty())
            channels.insert(key, it.value());
    }
    t_json.insert("channels", channels);
}

bool FixtureColorCalibration::isValid() const
{
    return !hues.isEmpty();
}

void FixtureColorCalibration::readFromJson(const QJsonObject &t_json)
{
    hues.clear();
    for(const auto &value : t_json.value("hues").toArray())
    {
        ColorCalibrationEntry entry;
        entry.readFromJson(value.toObject());
        hues.append(entry);
    }
    std::sort(hues.begin(), hues.end(), [](const ColorCalibrationEntry &a, const ColorCalibrationEntry &b){
        return a.hueDegrees < b.hueDegrees;
    });

    coolWhite = ColorCalibrationEntry();
    warmWhite = ColorCalibrationEntry();
    if(t_json.contains("coolWhite"))
        coolWhite.readFromJson(t_json.value("coolWhite").toObject());
    if(t_json.contains("warmWhite"))
        warmWhite.readFromJson(t_json.value("warmWhite").toObject());
}

void FixtureColorCalibration::writeToJson(QJsonObject &t_json) const
{
    QJsonArray huesArray;
    for(const auto &entry : hues)
    {
        QJsonObject obj;
        entry.writeToJson(obj);
        huesArray.append(obj);
    }
    t_json.insert("hues", huesArray);

    if(coolWhite.isValid())
    {
        QJsonObject obj;
        coolWhite.writeToJson(obj);
        t_json.insert("coolWhite", obj);
    }
    if(warmWhite.isValid())
    {
        QJsonObject obj;
        warmWhite.writeToJson(obj);
        t_json.insert("warmWhite", obj);
    }
}

namespace {

QMap<CapabilityType, double> lerpPercents(const QMap<CapabilityType, double> &t_a, const QMap<CapabilityType, double> &t_b, double t_fraction)
{
    QMap<CapabilityType, double> result;
    for(auto it = t_a.cbegin(); it != t_a.cend(); ++it)
        result.insert(it.key(), it.value());
    for(auto it = t_b.cbegin(); it != t_b.cend(); ++it)
        if(!result.contains(it.key()))
            result.insert(it.key(), 0.0);

    for(auto it = result.begin(); it != result.end(); ++it)
    {
        const double a = t_a.value(it.key(), 0.0);
        const double b = t_b.value(it.key(), 0.0);
        it.value() = a + (b - a) * t_fraction;
    }
    return result;
}

QMap<CapabilityType, double> scalePercents(const QMap<CapabilityType, double> &t_percents, double t_scale)
{
    QMap<CapabilityType, double> result;
    for(auto it = t_percents.cbegin(); it != t_percents.cend(); ++it)
        result.insert(it.key(), qBound(0.0, it.value() * t_scale, 1.0));
    return result;
}

} // namespace

QMap<CapabilityType, double> resolveCalibratedPercents(const FixtureColorCalibration &t_calibration, const QColor &t_color)
{
    if(!t_calibration.isValid())
        return {};

    QVector<ColorCalibrationEntry> hues = t_calibration.hues;
    std::sort(hues.begin(), hues.end(), [](const ColorCalibrationEntry &a, const ColorCalibrationEntry &b){
        return a.hueDegrees < b.hueDegrees;
    });

    int h = 0, s = 0, v = 0;
    t_color.getHsv(&h, &s, &v);
    const double saturation = s / 255.0;
    const double value = v / 255.0;

    QMap<CapabilityType, double> chromatic;

    if(h < 0)
    {
        // Fully achromatic input - nothing to interpolate by hue.
        chromatic = t_calibration.coolWhite.isValid() ? t_calibration.coolWhite.channelPercents : hues.first().channelPercents;
    }
    else if(hues.length() == 1)
    {
        chromatic = hues.first().channelPercents;
    }
    else
    {
        const double hueDegrees = h * (360.0 / 359.0);

        int upperIndex = 0;
        while(upperIndex < hues.length() && hues[upperIndex].hueDegrees < hueDegrees)
            ++upperIndex;

        const ColorCalibrationEntry &upper = hues[upperIndex % hues.length()];
        const ColorCalibrationEntry &lower = hues[(upperIndex + hues.length() - 1) % hues.length()];

        double span = upper.hueDegrees - lower.hueDegrees;
        if(span <= 0.0)
            span += 360.0;

        double delta = hueDegrees - lower.hueDegrees;
        if(delta < 0.0)
            delta += 360.0;

        const double fraction = span > 0.0 ? qBound(0.0, delta / span, 1.0) : 0.0;
        chromatic = lerpPercents(lower.channelPercents, upper.channelPercents, fraction);
    }

    const QMap<CapabilityType, double> mixed = t_calibration.coolWhite.isValid()
        ? lerpPercents(t_calibration.coolWhite.channelPercents, chromatic, saturation)
        : chromatic;

    return scalePercents(mixed, value);
}

FixtureColorCalibration FixtureColorCalibrationStore::load(const QString &t_definitionPath)
{
    FixtureColorCalibration calibration;
    if(t_definitionPath.isEmpty())
        return calibration;

    const QString hash = QString::fromLatin1(QCryptographicHash::hash(t_definitionPath.toUtf8(), QCryptographicHash::Sha1).toHex());
    QFile file(photonApp->appDataPath() + "/colorCalibration/" + hash + ".json");
    if(!file.open(QIODevice::ReadOnly))
        return calibration;

    calibration.readFromJson(QJsonDocument::fromJson(file.readAll()).object());
    return calibration;
}

void FixtureColorCalibrationStore::save(const QString &t_definitionPath, const FixtureColorCalibration &t_calibration)
{
    if(t_definitionPath.isEmpty())
        return;

    const QString dirPath = photonApp->appDataPath() + "/colorCalibration";
    QDir().mkpath(dirPath);

    const QString hash = QString::fromLatin1(QCryptographicHash::hash(t_definitionPath.toUtf8(), QCryptographicHash::Sha1).toHex());
    QFile file(dirPath + "/" + hash + ".json");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;

    QJsonObject obj;
    t_calibration.writeToJson(obj);
    file.write(QJsonDocument(obj).toJson());
}

} // namespace photon
