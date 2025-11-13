#ifndef PHOTON_DMXMATRIX_H
#define PHOTON_DMXMATRIX_H
#include "fixture/fixturechannel.h"

namespace photon {

class DMXMatrix
{
public:
    DMXMatrix(uint t_universeCount = 1, double t_currentTime = 0.0)
    {
        channels.resize(t_universeCount);
    }

    void blend(uchar &t_current, uchar t_target, double t_blend)
    {
        int current = t_current;
        t_current = ((t_target - current) * t_blend) + current;
    }

    double blendDouble(double t_current, double t_target, double t_blend)
    {
        double current = t_current;
        if(t_blend > 1.0)
            t_blend = 1.0;
        else if (t_blend < 0.0)
            t_blend = 0.0;
        return ((t_target - current) * t_blend) + current;
    }

    void setValue(uint t_universe, uint t_channel, uchar t_value, double t_blend = 1.0)
    {
        if(t_universe < channels.size() && t_channel < 512)
            blend(channels[t_universe][t_channel], t_value, t_blend);
    }

    void setRangeMappedValuePercent(FixtureChannel *t_channel, double t_value,DMXRange t_range)
    {
        setRangeMappedValuePercent(t_channel, t_value, t_range.start, t_range.end);
    }

    void setRangeMappedValuePercent(FixtureChannel *t_channel, double t_value,uchar t_min,uchar t_max)
    {
        double delta = t_max - t_min;
        delta *= t_value;
        delta += t_min;
        if(delta > 255)
            delta = 255;
        if(delta < 0)
            delta = 0;
        uint universe = t_channel->universe() - 1;
        uint channel = t_channel->universalChannelNumber();
        if(universe < channels.size() && channel < 512)
            blend(channels[universe][channel], delta, 1.0);

        if(t_channel->fineChannels().length() > 0 && t_channel->fineChannels()[0]->isValid())
        {
            double remainder = (delta * 255.0) - floor(delta * 255.0);

            uint fineUniverse = t_channel->fineChannels()[0]->universe() - 1;
            uint fineChannel = t_channel->fineChannels()[0]->universalChannelNumber();
            if(fineUniverse < channels.size() && fineChannel < 512)
                blend(channels[fineUniverse][fineChannel], floor(remainder * 255.0), 1.0);
        }


    }
    void setValuePercent(FixtureChannel *t_channel, double t_value, double t_blend = 1.0)
    {
        double currentValue = valuePercent(t_channel);
        double newValue = blendDouble(currentValue, t_value, t_blend);


        uint universe = t_channel->universe() - 1;
        uint channel = t_channel->universalChannelNumber();
        if(universe < channels.size() && channel < 512)
            blend(channels[universe][channel], floor(newValue * 255.0), 1.0);
        if(t_channel->fineChannels().length() > 0 && t_channel->fineChannels()[0]->isValid())
        {
            double remainder = (newValue * 255.0) - floor(newValue * 255.0);

            uint fineUniverse = t_channel->fineChannels()[0]->universe() - 1;
            uint fineChannel = t_channel->fineChannels()[0]->universalChannelNumber();
            if(fineUniverse < channels.size() && fineChannel < 512)
                blend(channels[fineUniverse][fineChannel], floor(remainder * 255.0), 1.0);
        }

    }
    void setValuePercent(uint t_universe, uint t_channel, double t_value, double t_blend = 1.0)
    {
        if(t_universe < channels.size() && t_channel < 512)
            blend(channels[t_universe][t_channel], floor(t_value * 255.0), t_blend);
    }
    void setValueIntFloor(uint t_universe, uint t_channel, int t_value, double t_blend = 1.0)
    {
        if(t_universe < channels.size() && t_channel < 512)
            blend(channels[t_universe][t_channel], std::min(std::max(t_value,0), 255), t_blend);
    }
    uchar value(uint t_universe, uint t_channel) const
    {
        if(t_universe < channels.size() && t_channel < 512)
            return channels[t_universe][t_channel];
        return 0;
    }
    double valuePercent(FixtureChannel *t_channel) const
    {
        double result = 0.0;
        uint universe = t_channel->universe() - 1;
        uint channel = t_channel->universalChannelNumber();
        if(universe < channels.size() && channel < 512)
            result = channels[universe][channel] / 255.0;

        if(t_channel->fineChannels().length() > 0 && t_channel->fineChannels()[0]->isValid())
        {
            uint fineUniverse = t_channel->fineChannels()[0]->universe() - 1;
            uint fineChannel = t_channel->fineChannels()[0]->universalChannelNumber();
            if(fineUniverse < channels.size() && fineChannel < 512)
                result = ((channels[universe][channel] + (channels[fineUniverse][fineChannel]/255.0) )/ 255.0);
        }

        return result;
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
