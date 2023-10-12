#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QAudioBuffer>
#include "waveformwidget.h"

// Standard headers
#include <limits.h>
#include <math.h>
#include <stdlib.h>



struct SampleBlock
{
    enum { MAX_SAMPLES = 131072 };
    enum { SAMPLES_PER_LUT_ITEM = 256 };	// For optimal results, set this to sqrt(sample_len / screen_width_in_pixels), rounded to the nearest power of 2.
    enum { LUT_SIZE = MAX_SAMPLES / SAMPLES_PER_LUT_ITEM };

    int16_t     m_samples[MAX_SAMPLES];
    unsigned    m_len;   // Number of valid items in m_samples
    int16_t     m_maxLut[LUT_SIZE];
    int16_t     m_minLut[LUT_SIZE];

    SampleBlock(){}

    void RecalcLuts(){
        int16_t *currentSample = m_samples;
        int16_t *lastSample = m_samples + m_len;
        for (unsigned i = 0; i < LUT_SIZE; i++)
        {
            int16_t _min = INT16_MAX;
            int16_t _max = INT16_MIN;
            for (unsigned j = 0; j < SAMPLES_PER_LUT_ITEM; j++)
            {
                if (currentSample >= lastSample)
                    break;
                //                _min = std::min(*currentSample, _min);
                //                _max = std::max(*currentSample, _max);
                _min = std::min(*currentSample, _min);
                _max = std::max(*currentSample, _max);
                currentSample++;
            }

            m_maxLut[i] = _max;
            m_minLut[i] = _min;
        }
    }
};


class SoundChannel
{
public:
    ~SoundChannel()
    {
        for(auto block : m_blocks)
            delete block;
    }

    struct SoundPos
    {
        int m_blockIdx;
        int m_sampleIdx;

        SoundPos()
        {
            m_blockIdx = 0;
            m_sampleIdx = 0;
        }

        SoundPos(int blockIdx, int sampleIdx)
        {
            m_blockIdx = blockIdx;
            m_sampleIdx = sampleIdx;
        }
    };

private:
    void CalcMinMaxForRange(SoundPos *pos, unsigned numSamples, int16_t *resultMin, int16_t *resultMax)
    {
        // If pos is entirely outside the bounds of the sample, then we set resultMin and resultMax to zero.
        if (pos->m_blockIdx >= m_blocks.size())
        {
            *resultMax = 0;
            *resultMin = 0;
            return;
        }

        SampleBlock *block = m_blocks[pos->m_blockIdx];

        int16_t _min = INT16_MAX;
        int16_t _max = INT16_MIN;

        // Do the slow bit at the start of the range.
        {
            const unsigned lutItemBoundaryMask = SampleBlock::SAMPLES_PER_LUT_ITEM - 1;
            unsigned numSamplesToNextLutItemBoundary = (SampleBlock::SAMPLES_PER_LUT_ITEM - pos->m_sampleIdx) & lutItemBoundaryMask;
            unsigned numSlowSamples = numSamplesToNextLutItemBoundary;
            if (numSlowSamples > numSamples)
                numSlowSamples = numSamples;
            if (numSlowSamples > block->m_len)
                numSlowSamples = block->m_len;

            int idx = pos->m_sampleIdx;
            int end_idx = idx + numSlowSamples;
            if (end_idx > static_cast<int>(block->m_len))
                end_idx = block->m_len;

            while (idx < end_idx)
            {
                _min = std::min(block->m_samples[idx], _min);
                _max = std::max(block->m_samples[idx], _max);
                idx++;
            }


            numSamples -= numSlowSamples;
            block = IncrementSoundPos(pos, numSlowSamples);
            //qDebug() << pos->m_blockIdx << pos->m_sampleIdx;
        }

        // Do the fast bit - one block per iteration.
        while (block && numSamples > SampleBlock::SAMPLES_PER_LUT_ITEM)
        {
            // Cases here:
            // 1. There is one or more LUT items in this block that we can use.
            // 2. There is one or more LUT items left, but the pixel doesn't span entirely over it/them.

            unsigned numLutItemsWeNeed = numSamples / SampleBlock::SAMPLES_PER_LUT_ITEM;
            int currentLutItemIdx = pos->m_sampleIdx / SampleBlock::SAMPLES_PER_LUT_ITEM;
            unsigned numLutItemsInThisBlock = ((block->m_len - 1) / SampleBlock::SAMPLES_PER_LUT_ITEM) + 1;
            unsigned numLutItemsLeftInThisBlock = numLutItemsInThisBlock - currentLutItemIdx;

            unsigned numLutItemsToUse = numLutItemsWeNeed;
            if (numLutItemsLeftInThisBlock < numLutItemsToUse)
                numLutItemsToUse = numLutItemsLeftInThisBlock;

            int endLutItemIdx = currentLutItemIdx + numLutItemsToUse;
            while (currentLutItemIdx < endLutItemIdx)
            {
                _min = std::min(block->m_minLut[currentLutItemIdx], _min);
                _max = std::max(block->m_maxLut[currentLutItemIdx], _max);
                currentLutItemIdx++;
            }

            // Calculate how many samples we processed. It's a little complex because the
            // LUT items we used might not have been "full", if the block has been part of
            // and insert or delete operation previously.
            unsigned numSamplesLeftInThisBlockBeforeWeDidTheFastBit = block->m_len - pos->m_sampleIdx;
            unsigned numSamplesProcessedThisIteration = numLutItemsToUse * SampleBlock::SAMPLES_PER_LUT_ITEM;
            if (numSamples > numSamplesLeftInThisBlockBeforeWeDidTheFastBit)
                numSamplesProcessedThisIteration = numSamplesLeftInThisBlockBeforeWeDidTheFastBit;

            numSamples -= numSamplesProcessedThisIteration;
            block = IncrementSoundPos(pos, numSamplesProcessedThisIteration);
        }

        // Do the slow bit at the end of the range.
        while (block && numSamples)
        {
            // We are not at end of block and numSamples is less than next LUT item boundary.
            // Cases:
            // 1 - There are enough samples in the current block to cover the current pixel.
            // 2 - There aren't.

            int idx = pos->m_sampleIdx;
            int end_idx = pos->m_sampleIdx + numSamples;
            if (end_idx > static_cast<int>(block->m_len))
                end_idx = block->m_len;

            int numSamplesThisIteration = end_idx - idx;

            while (idx < end_idx)
            {
                _min = std::min(block->m_samples[idx], _min);
                _max = std::max(block->m_samples[idx], _max);
                idx++;
            }

            block = IncrementSoundPos(pos, numSamplesThisIteration);
            numSamples -= numSamplesThisIteration;
        }

        *resultMin = _min;
        *resultMax = _max;
    }

public:
    SoundPos GetSoundPosFromSampleIdx(int64_t sampleIdx)
    {
        int blockIdx = 0;

        while (sampleIdx >= m_blocks[blockIdx]->m_len)
        {
            sampleIdx -= m_blocks[blockIdx]->m_len;
            blockIdx++;
            if (blockIdx > m_blocks.size())
                return SoundPos(-1, -1);
        }

        return SoundPos(blockIdx, sampleIdx);
    }
    SampleBlock *IncrementSoundPos(SoundPos *pos, int64_t numSamples)
    {
        if (pos->m_blockIdx >= m_blocks.size())
            return NULL;

        SampleBlock *block = m_blocks[pos->m_blockIdx];

        Q_ASSERT(pos->m_sampleIdx < static_cast<int>(block->m_len));

        // Iterate across blocks until we've crossed enough samples.
        while (numSamples > 0)
        {
            // Has this block got at least 'numSamples' left?
            if (pos->m_sampleIdx + numSamples < block->m_len)
            {
                // Yes
                pos->m_sampleIdx += numSamples;
                numSamples = 0;
            }
            else
            {
                // No
                numSamples -= block->m_len - pos->m_sampleIdx;
                pos->m_blockIdx++;
                if (pos->m_blockIdx >= m_blocks.size())
                    return NULL;

                block = m_blocks[pos->m_blockIdx];
                pos->m_sampleIdx = 0;
            }
        }

        return block;
    }

    // Each block has at most N samples (where N is probably 2^17). Any two adjacent blocks that total <= N samples will be merged.
    std::vector<SampleBlock *> m_blocks;

    unsigned GetLength(){
        unsigned len = 0;
        for (int i = 0; i < m_blocks.size(); i++)
            len += m_blocks[i]->m_len;
        return len;
    }

    void CalcDisplayData(int startSampleIdx, int16_t *mins, int16_t *maxes, unsigned widthInPixels, double samplesPerPixel)
    {
        SoundPos pos = GetSoundPosFromSampleIdx(startSampleIdx);

        double widthErrorPerPixel = samplesPerPixel - floorf(samplesPerPixel);
        double error = 0;
        for (unsigned x = 0; x < widthInPixels; x++)
        {
            if (pos.m_blockIdx < m_blocks.size())
            {
                int samplesThisPixel = samplesPerPixel;
                if (error > 1.0)
                {
                    samplesThisPixel++;
                    error -= 1.0;
                }
                error += widthErrorPerPixel;

                if(pos.m_sampleIdx < 0)
                {
                    if(pos.m_sampleIdx + samplesThisPixel < 0)
                    {
                        pos.m_sampleIdx += samplesThisPixel;
                        mins[x] = 0;
                        maxes[x] = 0;

                        continue;
                    }
                    else
                        pos.m_sampleIdx = 0;

                }

                CalcMinMaxForRange(&pos, samplesThisPixel, mins + x, maxes + x);

                if (x > 0)
                {
                    if (mins[x] > maxes[x - 1])
                        mins[x] = maxes[x - 1] + 1;
                    if (maxes[x] < mins[x - 1])
                        maxes[x] = mins[x - 1] - 1;
                }
            }
            else
            {
                mins[x] = 0;
                maxes[x] = 0;
            }
        }
    }
};


class Sound
{
private:
    int64_t m_cachedLength;

public:
    enum
    {
        ERROR_NO_ERROR,
        ERROR_WRONG_NUMBER_OF_CHANNELS
    };

    SoundChannel **m_channels;  // All the channels contain the same number of samples.
    int m_numChannels = 0;
    double m_samplesPerSecond = 0;
    qint64 m_duration = 0;

    Sound(){}
    ~Sound(){
        for (int i = 0; i < m_numChannels; i++)
            delete m_channels[i];
    }

    void read(const QAudioBuffer& t_buffer)
    {
        if(m_numChannels == 0)// initialize
        {
            m_numChannels = t_buffer.format().channelCount();
            m_channels = new SoundChannel* [m_numChannels];
                for (int i = 0; i < m_numChannels; i++)
                    m_channels[i] = new SoundChannel;

        }
        const qint16 *buf = t_buffer.data<qint16>();

        m_duration += t_buffer.duration();


        unsigned bytesPerGroup = t_buffer.format().bytesPerFrame();

        if(bytesPerGroup == 0)
            return;

        unsigned dataChunkSize = t_buffer.byteCount();

        unsigned numGroups = dataChunkSize / bytesPerGroup;
        unsigned numBlocks = numGroups / SampleBlock::MAX_SAMPLES;
        if (numGroups % SampleBlock::MAX_SAMPLES != 0)
            numBlocks++;

        if(m_numChannels == 0)
        {
            m_channels = new SoundChannel* [m_numChannels];
            for (int i = 0; i < m_numChannels; i++)
                m_channels[i] = new SoundChannel;
        }



        for (unsigned blockCount = 0; blockCount < numBlocks; blockCount++)
        {
            unsigned bytesToRead = bytesPerGroup * SampleBlock::MAX_SAMPLES;
            if (blockCount + 1 == numBlocks)
                bytesToRead = (numGroups % SampleBlock::MAX_SAMPLES) * bytesPerGroup;
            uint bytesRead = bytesToRead;
            uint groupsRead = bytesRead / bytesPerGroup;

            for (int chan_idx = 0; chan_idx < m_numChannels; chan_idx++)
            {
                SoundChannel *chan = m_channels[chan_idx];
                SampleBlock *block = new SampleBlock;

                for (size_t i = 0; i < groupsRead; i++)
                {
                    block->m_samples[i] = buf[i * m_numChannels + chan_idx];
                }

                block->m_len = groupsRead;
                block->RecalcLuts();

                chan->m_blocks.push_back(block);

            }
        }

    }

    void Complete()
    {
        m_samplesPerSecond = GetLength() / (m_duration/1000000.0);
    }

    int64_t GetLength()
    {
        if (m_numChannels == 0)
                return 0;

        if (m_cachedLength < 0)
            m_cachedLength = m_channels[0]->GetLength();

        return m_cachedLength;
    }
};






WaveformWidget::WaveformWidget(QWidget *parent)
    : QWidget{parent},
    m_decoder(new QAudioDecoder(this))
{
    m_sound = nullptr;
    m_displayMins = nullptr;
    m_displayMaxes = nullptr;
    setMouseTracking(true);

    m_backgroundColor = QColor(64,64,64);
    m_waveformColor = QColor(85,102,160);
    m_centerLineColor = QColor(200,200,200);
    m_selectionColor = QColor(0,255,255,80);
    m_playheadColor = Qt::red;

    /*
    QAudioFormat desiredFormat;
    desiredFormat.setChannelCount(1);
    desiredFormat.setSampleFormat(QAudioFormat::UInt8);
    m_decoder->setAudioFormat(desiredFormat);
    */


    connect(m_decoder, &QAudioDecoder::bufferReady, this, &WaveformWidget::bufferReady);
    connect(m_decoder, &QAudioDecoder::finished, this, &WaveformWidget::finished);
}

WaveformWidget::~WaveformWidget()
{
    delete m_sound;
}

void WaveformWidget::setBackgroundColor(const QColor &t_color)
{
    if(m_backgroundColor == t_color)
        return;
    m_backgroundColor = t_color;
    update();
}

void WaveformWidget::setWaveformColor(const QColor &t_color)
{
    if(m_waveformColor == t_color)
        return;
    m_waveformColor = t_color;
    update();

}

void WaveformWidget::setCenterLineColor(const QColor &t_color)
{
    if(m_centerLineColor == t_color)
        return;
    m_centerLineColor = t_color;
    update();
}

void WaveformWidget::setPlayheadColor(const QColor &t_color)
{
    if(m_playheadColor == t_color)
        return;
    m_playheadColor = t_color;

    if(m_showPlayhead)
        update();
}

void WaveformWidget::setSelectionColor(const QColor &t_color)
{
    if(m_selectionColor == t_color)
        return;
    m_selectionColor = t_color;

    if(m_showSelection)
        update();
}


void WaveformWidget::loadAudio(const QString &path)
{
    if(m_sound)
        delete m_sound;

    m_isRenderable = false;
    m_sound = new Sound;

    m_decoder->setSource(path);
    m_decoder->start();
    update();
}

void WaveformWidget::clearAudio()
{
    if(m_sound)
        delete m_sound;

    m_isRenderable = false;
    m_sound = nullptr;
    update();
}

void WaveformWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter{this};

    painter.fillRect(event->rect(), m_backgroundColor);


    if (!m_isRenderable) return;

    if (m_hZoomRatio < 0.0)
    {
        m_hZoomRatio = (double)m_sound->GetLength() / (double)width();
        m_targetHZoomRatio = m_hZoomRatio;
    }


    double vZoomRatio = (double)height() / (65536 * m_sound->m_numChannels);

    if(m_redrawWaveform)
    {
        m_cachedWaveform.fill(Qt::transparent);
        QPainter waveformPainter{&m_cachedWaveform};
        RenderWaveform(&waveformPainter, vZoomRatio);
        m_redrawWaveform = false;
    }
    painter.drawImage(0,0,m_cachedWaveform);

    if(m_showSelection)
    {
        double x = timeToX(m_selectionRange.start);
        painter.fillRect(x,0,timeToX(m_selectionRange.end) - x,height(), m_selectionColor);
    }

    if(m_showPlayhead)
    {
        painter.fillRect(timeToX(m_playheadTime),0,1,height(),m_playheadColor);
    }

}


void WaveformWidget::frameAll()
{
    if(!m_sound)
        return;

    frameTime(0, m_sound->GetLength() / m_sound->m_samplesPerSecond);
}

void WaveformWidget::frameTime(double start, double end)
{
    if(end < start)
        std::swap(start, end);

    m_visibleRange.start = start;
    m_visibleRange.end = end;

    if(!m_sound)
        return;

    double w = end - start;
    m_redrawWaveform = true;
    m_hZoomRatio = (w * m_sound->m_samplesPerSecond) / (double)width();
    m_hOffset = start * m_sound->m_samplesPerSecond;
    update();
}

void WaveformWidget::bufferReady()
{
    auto buffer = m_decoder->read();

    m_sound->read(buffer);
}

void WaveformWidget::finished()
{
    m_sound->Complete();
    m_redrawWaveform = true;
    m_isRenderable = true;

    if(m_visibleRange.end == 0)
        frameAll();
    else
        frameTime(m_visibleRange.start, m_visibleRange.end);

}

void WaveformWidget::resizeEvent(QResizeEvent *t_event)
{
    if (m_displayMins != nullptr)
    {
        delete[] m_displayMins;
        delete[] m_displayMaxes;
    }

    m_displayMins = new int16_t[t_event->size().width()];
    m_displayMaxes = new int16_t[t_event->size().width()];

    m_redrawWaveform = true;
    m_cachedWaveform = QImage(t_event->size(), QImage::Format_ARGB32_Premultiplied);

}

double WaveformWidget::xToTime(int t_x) const
{
    if(!m_sound)
        return 0.0;
    return ((t_x *  m_hZoomRatio) + m_hOffset)/m_sound->m_samplesPerSecond;
}

double WaveformWidget::timeToX(double time) const
{
    if(!m_sound)
        return 0.0;
    return ((time * m_sound->m_samplesPerSecond) - m_hOffset) / m_hZoomRatio;
}

void WaveformWidget::setPlayhead(double time)
{
    m_playheadTime = time;
    m_showPlayhead = true;
    update();
}

void WaveformWidget::clearPlayhead()
{
    if(!m_showPlayhead)
        return;
    m_showPlayhead = false;
    update();
}

void WaveformWidget::setSelectionRange(double start, double end)
{
    m_selectionRange.start = start;
    m_selectionRange.end = end;
    m_showSelection = true;
    update();
}

void WaveformWidget::clearSelection()
{
    if(!m_showSelection)
        return;
    m_showSelection = false;
    update();
}


void WaveformWidget::mousePressEvent(QMouseEvent *t_event)
{
    m_lastPosition = t_event->pos();
    if(t_event->button() == Qt::MiddleButton)
        m_isDragging = true;

    if(t_event->modifiers() == Qt::NoModifier)
    {

        //setPlayhead(xToTime(t_event->pos().x()));
    }

}

void WaveformWidget::mouseMoveEvent(QMouseEvent *t_event)
{
    if(m_isDragging)
    {
        QPoint delta = t_event->pos() - m_lastPosition;

        m_hOffset += -delta.x() * m_hZoomRatio;
        m_redrawWaveform = true;

        update();
    }
    m_lastPosition = t_event->pos();

}

void WaveformWidget::mouseReleaseEvent(QMouseEvent *t_event)
{
    m_isDragging = false;
}

void WaveformWidget::wheelEvent(QWheelEvent *t_event)
{
    double underCursor = t_event->position().x() * m_hZoomRatio;
    double const ZOOM_INCREMENT = 1.3;

    if(t_event->angleDelta().y() > 0)
        m_hZoomRatio *= ZOOM_INCREMENT;
    else
        m_hZoomRatio /= ZOOM_INCREMENT;

    double nowUnderCursor = t_event->position().x() * m_hZoomRatio;

    m_hOffset -= (nowUnderCursor - underCursor);
    m_redrawWaveform = true;

    update();
}

void WaveformWidget::RenderWaveform(QPainter *t_painter, double v_zoom_ratio)
{
    int channelHeight = height() / m_sound->m_numChannels;

    for (int chanIdx = 0; chanIdx < m_sound->m_numChannels; chanIdx++)
    {
        SoundChannel *chan = m_sound->m_channels[chanIdx];

        int yMid = rect().top() + channelHeight * chanIdx + channelHeight / 2;
        if( m_hOffset < chan->GetLength())
        {
            chan->CalcDisplayData(m_hOffset, m_displayMins, m_displayMaxes, width(), m_hZoomRatio);


            for (int x = 0; x < width(); x++)
            {
                //qDebug() << x << m_displayMaxes[x] << m_displayMins[x] << v_zoom_ratio;
                int vline_len = std::ceil((m_displayMaxes[x] - m_displayMins[x]) * v_zoom_ratio);
                int y = ceil(yMid - m_displayMaxes[x] * v_zoom_ratio);
                if (vline_len == 0)
                    t_painter->fillRect(QRect(rect().left() + x, y,1,1), m_waveformColor);
                else
                    t_painter->fillRect(QRect(rect().left() + x, y, 1, vline_len), m_waveformColor);
            }
        }

        t_painter->fillRect(QRect(rect().left(), yMid,  width(), 1), m_centerLineColor);
        t_painter->fillRect(QRect(rect().left(), yMid+ 32767 * v_zoom_ratio,  width(), 1), m_centerLineColor);
    }
}
