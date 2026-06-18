#include "dmxtimemachine.h"
#include "data/dmxmatrix.h"

namespace photon {

void DMXFrameData::writeData(DMXMatrix *t_matrix)
{
    if(fixtureChannel)
    {
        switch (mode) {
        case MODE_SET_VALUE:
            break;
        case MODE_SET_VALUE_PERCENT:
            t_matrix->setValuePercent(fixtureChannel, value, blend);
            break;
        case MODE_SET_RANGE_MAPPED_PERCENT:
            t_matrix->setRangeMappedValuePercent(fixtureChannel, value, rangeMin, rangeMax);
            break;

        default:
            break;
        }
    }
    else{
        switch (mode) {
            case MODE_SET_VALUE:
                t_matrix->setValue(universe, channel, value, blend);
                break;
            case MODE_SET_VALUE_PERCENT:
                t_matrix->setValuePercent(universe, channel, value, blend);
                break;
            case MODE_SET_RANGE_MAPPED_PERCENT:
                t_matrix->setValuePercent(universe, channel, value, blend);
                break;

            default:
                break;
        }
    }


}

DMXTimeMachine::DMXTimeMachine() {}

void DMXTimeMachine::initializeMatrix(DMXMatrix *matrix, qlonglong t_frame)
{
    m_matrix = matrix;
    m_frame = t_frame;
}

void DMXTimeMachine::setTargetFrame(qlonglong t_frame)
{
    m_targetFrame = t_frame;
}

void DMXTimeMachine::storeData(DMXFrameData data)
{
    if(!m_matrix)
        return;


    if(m_targetFrame <= m_frame)
    {
        data.writeData(m_matrix);
    }
    else
    {
        for(auto frameIt = m_frames.begin(); frameIt != m_frames.end(); frameIt++)
        {
            if(m_targetFrame == (*frameIt).frame)
            {
                (*frameIt).addData(data);
                return;
            }
            else if ((*frameIt).frame > m_targetFrame)
            {
                auto frame = DMXFrame(m_targetFrame);
                frame.addData(data);
                m_frames.insert(frameIt,frame);
                return;
            }
        }

        auto frame = DMXFrame(m_targetFrame);
        frame.addData(data);
        m_frames.append(frame);
    }

}

void DMXTimeMachine::writeStoredData()
{
    if(m_frames.isEmpty() || !m_matrix)
        return;

    int counter = 0;
    //qDebug() << "Begin" << m_frame;
    for(auto frameIt = m_frames.begin(); frameIt != m_frames.end(); frameIt++)
    {
        if((*frameIt).frame <= m_frame)
        {
            for(auto &data : (*frameIt).data)
            {
                data.writeData(m_matrix);
            }

            counter++;


            //qDebug() << "process" << (*frameIt).frame;
        }
        else
        {
            if(counter > 0)
                m_frames.remove(0,counter);
            return;
        }
    }
    //qDebug() << "none";

    if(counter > 0)
        m_frames.remove(0,counter);
    return;

}

void DMXTimeMachine::releaseMatrix()
{
    m_matrix = nullptr;
    m_frame = 0;
}


} // namespace photon
