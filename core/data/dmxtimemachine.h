#ifndef DMXTIMEMACHINE_H
#define DMXTIMEMACHINE_H
#include <QObject>
#include "photon-global.h"

namespace photon {

struct DMXFrameData
{
    enum Mode{
        MODE_SET_VALUE,
        MODE_SET_RANGE_MAPPED_PERCENT,
        MODE_SET_VALUE_PERCENT
    };


    DMXFrameData(){}
    DMXFrameData(uint universe, uint channel, uchar value, float blend = 1.0f, Mode mode = MODE_SET_VALUE):universe(universe),channel(channel),value(value),blend(blend),mode(mode){}
    DMXFrameData(FixtureChannel* channel, float value, float blend = 1.0f, Mode mode = MODE_SET_VALUE):fixtureChannel(channel),value(value),blend(blend),mode(mode){}
    DMXFrameData(FixtureChannel* channel, float value, float blend = 1.0f,  uchar min = 0, uchar max = 255):fixtureChannel(channel),value(value),blend(blend),mode(MODE_SET_RANGE_MAPPED_PERCENT){}
    void writeData(DMXMatrix *matrix);
    FixtureChannel *fixtureChannel = nullptr;
    uint universe;
    uint channel;
    uchar rangeMin;
    uchar rangeMax;
    float value;
    float blend;
    Mode mode;
};

class DMXFrame{
public:
    DMXFrame(qlonglong frame):frame(frame){}

    void addData(uint universe, uint channel, uchar value, float blend){
        data.append(DMXFrameData{universe, channel, value, blend});
    }
    void addData(DMXFrameData &d)
    {
        data.append(d);
    }

    qlonglong frame;
    QVector<DMXFrameData> data;
};


class DMXTimeMachine
{
public:
    DMXTimeMachine();

    void initializeMatrix(DMXMatrix *matrix, qlonglong frame);
    void setTargetFrame(qlonglong);
    void storeData(DMXFrameData data);
    void writeStoredData();
    void releaseMatrix();


private:
    QVector<DMXFrame> m_frames;
    DMXMatrix *m_matrix = nullptr;
    qlonglong m_targetFrame = 0;
    qlonglong m_frame = 0;
};

} // namespace photon

#endif // DMXTIMEMACHINE_H
