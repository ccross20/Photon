#include <cmath>
#include <QHash>
#include "lasercapability.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/fixturechannel.h"

namespace photon {

static LaserCapability::Function functionFromString(const QString &t_name)
{
    static const QHash<QString, LaserCapability::Function> functions = {
        {"mode", LaserCapability::Function_Mode},
        {"setupintensity", LaserCapability::Function_SetupIntensity},
        {"setuptestframe", LaserCapability::Function_SetupTestFrame},
        {"setupsizex", LaserCapability::Function_SetupSizeX},
        {"setupsizey", LaserCapability::Function_SetupSizeY},
        {"setuppositionx", LaserCapability::Function_SetupPositionX},
        {"setuppositiony", LaserCapability::Function_SetupPositionY},
        {"setuprotation", LaserCapability::Function_SetupRotation},
        {"page", LaserCapability::Function_Page},
        {"cue", LaserCapability::Function_Cue},
        {"cuespeed", LaserCapability::Function_CueSpeed},
        {"zoom", LaserCapability::Function_Zoom},
        {"sizex", LaserCapability::Function_SizeX},
        {"sizey", LaserCapability::Function_SizeY},
        {"angle", LaserCapability::Function_Angle},
        {"rotation", LaserCapability::Function_Rotation},
        {"positionx", LaserCapability::Function_PositionX},
        {"positiony", LaserCapability::Function_PositionY},
        {"scanrate", LaserCapability::Function_ScanRate},
        {"red", LaserCapability::Function_Red},
        {"green", LaserCapability::Function_Green},
        {"blue", LaserCapability::Function_Blue},
        {"colormix", LaserCapability::Function_ColorMix},
        {"visiblestart", LaserCapability::Function_VisibleStart},
        {"visibleend", LaserCapability::Function_VisibleEnd},
        {"strobe", LaserCapability::Function_Strobe},
    };
    return functions.value(t_name.toLower(), LaserCapability::Function_Unknown);
}

LaserCapability::LaserCapability() : FixtureCapability(DMXRange{}, Capability_Laser)
{
}

LaserCapability::Function LaserCapability::function() const
{
    return m_function;
}

bool LaserCapability::is16Bit() const
{
    const auto &fine = channel()->fineChannels();
    return !fine.isEmpty() && fine.first()->isValid();
}

int LaserCapability::raw(const DMXMatrix &t_matrix) const
{
    FixtureChannel *coarse = channel();
    if(!coarse->isValid())
        return 0;

    const int value = t_matrix.valueInt(coarse->universe() - 1, coarse->universalChannelNumber());
    if(!is16Bit())
        return value;

    FixtureChannel *fine = coarse->fineChannels().first();
    return (value << 8) | t_matrix.valueInt(fine->universe() - 1, fine->universalChannelNumber());
}

void LaserCapability::setRaw(int t_value, DMXMatrix &t_matrix, double t_blend) const
{
    FixtureChannel *coarse = channel();
    if(!coarse->isValid())
        return;

    const bool wide = is16Bit();
    const int maximum = wide ? 65535 : 255;
    int target = std::clamp(t_value, 0, maximum);

    // Blend on the whole word: blending coarse and fine bytes separately
    // would sweep the fine byte through its full range on every coarse step.
    if(t_blend < 1.0)
    {
        const int current = raw(t_matrix);
        target = int(std::lround(current + (target - current) * std::clamp(t_blend, 0.0, 1.0)));
    }

    if(!wide)
    {
        t_matrix.setValue(coarse->universe() - 1, coarse->universalChannelNumber(), uchar(target));
        return;
    }

    FixtureChannel *fine = coarse->fineChannels().first();
    t_matrix.setValue(coarse->universe() - 1, coarse->universalChannelNumber(), uchar(target >> 8));
    t_matrix.setValue(fine->universe() - 1, fine->universalChannelNumber(), uchar(target & 0xFF));
}

void LaserCapability::setCentered(double t_value, DMXMatrix &t_matrix, double t_blend) const
{
    const double v = std::clamp(t_value, -1.0, 1.0);
    if(is16Bit())
    {
        // Split around 32768 so 0 lands exactly on the center word.
        const int word = v < 0.0 ? int(std::lround(32768.0 * (1.0 + v)))
                                 : int(std::lround(32768.0 + 32767.0 * v));
        setRaw(word, t_matrix, t_blend);
    }
    else
    {
        const int word = v < 0.0 ? int(std::lround(128.0 * (1.0 + v)))
                                 : int(std::lround(128.0 + 127.0 * v));
        setRaw(word, t_matrix, t_blend);
    }
}

void LaserCapability::setPercent(double t_value, DMXMatrix &t_matrix, double t_blend) const
{
    const int maximum = is16Bit() ? 65535 : 255;
    setRaw(int(std::lround(std::clamp(t_value, 0.0, 1.0) * maximum)), t_matrix, t_blend);
}

int LaserCapability::neutralRaw() const
{
    const bool wide = is16Bit();
    switch(m_function)
    {
    case Function_Zoom:
    case Function_SizeX:
    case Function_SizeY:
    case Function_PositionX:
    case Function_PositionY:
    case Function_SetupSizeX:
    case Function_SetupSizeY:
    case Function_SetupPositionX:
    case Function_SetupPositionY:
        return wide ? 32768 : 128;
    case Function_CueSpeed:
        return 100;
    case Function_ScanRate:
        return wide ? 65535 : 255;
    default:
        // Includes Rotation, where 0 means "use the cue's own rotation".
        return 0;
    }
}

void LaserCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);
    m_function = functionFromString(t_json.value("function").toString());
    if(m_function == Function_Unknown)
        qWarning() << "Unknown laser function" << t_json.value("function").toString();
}

LaserCapability *LaserCapability::find(const Fixture *t_fixture, Function t_function)
{
    if(!t_fixture)
        return nullptr;

    for(auto *capability : t_fixture->findCapability<LaserCapability*>())
    {
        if(capability->function() == t_function)
            return capability;
    }
    return nullptr;
}

void LaserCapability::writeNeutralValues(const Fixture *t_fixture, DMXMatrix &t_matrix)
{
    for(auto *capability : t_fixture->findCapability<LaserCapability*>())
        capability->setRaw(capability->neutralRaw(), t_matrix);
}

} // namespace photon
