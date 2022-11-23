#ifndef PHOTON_DMXMATRIX_H
#define PHOTON_DMXMATRIX_H
#include "photon-global.h"

namespace photon {

class DMXMatrix
{
public:
    DMXMatrix(uint t_universeCount = 1, double t_currentTime = 0.0)
    {
        channels.resize(t_universeCount);
    }
    void setValue(uint t_universe, uint t_channel, uchar t_value)
    {
        if(t_universe < channels.size() && t_channel < 512)
            channels[t_universe][t_channel] = t_value;
    }
    void setValuePercent(uint t_universe, uint t_channel, double t_value)
    {
        if(t_universe < channels.size() && t_channel < 512)
            channels[t_universe][t_channel] = floor(t_value * 255.0);
    }
    void setValueIntFloor(uint t_universe, uint t_channel, int t_value)
    {
        if(t_universe < channels.size() && t_channel < 512)
            channels[t_universe][t_channel] = std::min(std::max(t_value,0), 255);
    }
    uchar value(uint t_universe, uint t_channel) const
    {
        if(t_universe < channels.size() && t_channel < 512)
            return channels[t_universe][t_channel];
        return 0;
    }
    double valuePercent(uint t_universe, uint t_channel) const
    {
        if(t_universe < channels.size() && t_channel < 512)
            return channels[t_universe][t_channel] / 255.0;
        return 0.0;
    }
    int valueInt(uint t_universe, uint t_channel) const
    {
        if(t_universe < channels.size() && t_channel < 512)
            return channels[t_universe][t_channel];
        return 0;
    }

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

    friend QDebug operator<< (QDebug debug, const DMXMatrix &t_matrix)
    {
        QDebugStateSaver saver(debug);
        debug.nospace() << "DMX Matrix " << t_matrix.channels.size() << " Universes)";

        return debug;
    }

    friend QDataStream & operator<< (QDataStream& stream, const DMXMatrix &t_matrix)
    {
        stream << static_cast<uchar>(1); // version number
        stream << t_matrix.channels.size();
        for(auto universeIt = t_matrix.channels.cbegin(); universeIt != t_matrix.channels.cend(); ++universeIt)
        {
            const auto &universe = *universeIt;
            for(auto channelIt = universe.cbegin(); channelIt != universe.cend(); ++channelIt)
                stream << *channelIt;

        }
        return stream;
    }
    friend QDataStream & operator>> (QDataStream& stream, DMXMatrix &t_matrix)
    {
        uchar version = 0;
        size_t universeCount = 0;
        stream >> version;
        stream >> universeCount;

        t_matrix.channels.resize(universeCount);

        for(auto universeIt = t_matrix.channels.begin(); universeIt != t_matrix.channels.cend(); ++universeIt)
        {
            auto &universe = *universeIt;
            for(auto channelIt = universe.begin(); channelIt != universe.end(); ++channelIt)
                stream >> *channelIt;

        }

        return stream;
    }

    std::vector<std::array<uchar, 512>> channels;

};


} // namespace photon


Q_DECLARE_METATYPE(photon::DMXMatrix)

#endif // PHOTON_DMXMATRIX_H
