#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>
#include "sequencewaveformeditor.h"
#include "sequence/sequence.h"
#include "sequence/beatlayer.h"
#include "audio/songdata.h"

namespace photon {

class SequenceWaveformEditor::Impl
{
public:
    enum DragMode{
        DragNone,
        DragMove,
        DragSelect
    };
    void clearBeats();

    Sequence *sequence;
    QVector<float> otherBeats;
    QVector<float> selectedBeats;
    QVector<float> selectedBeatsInitial;
    BeatLayer *editableLayer = nullptr;
    DragMode dragMode = DragNone;
    double initialTime = 0;
    Range selectionRange;
};

void SequenceWaveformEditor::Impl::clearBeats()
{
    selectedBeats.clear();
    otherBeats.clear();
    editableLayer = sequence->editableBeatLayer();
    if(editableLayer)
        otherBeats = editableLayer->beats();
}

SequenceWaveformEditor::SequenceWaveformEditor(Sequence *t_sequence, QWidget *t_parent): WaveformWidget(t_parent),m_impl(new Impl)
{
    m_impl->sequence = t_sequence;
    setFocusPolicy(Qt::StrongFocus);

    // The layered low/mid/high band overlay (see drawFeatureOverlay) replaces the
    // raw audio waveform as this view's primary visual.
    setShowWaveform(false);
}

SequenceWaveformEditor::~SequenceWaveformEditor()
{
    delete m_impl;
}

Sequence *SequenceWaveformEditor::sequence() const
{
    return m_impl->sequence;
}

void SequenceWaveformEditor::setSequence(Sequence *t_sequence)
{
    m_impl->sequence = t_sequence;
    if(t_sequence)
    {
        connect(m_impl->sequence, &Sequence::editableBeatLayerChanged, this, &SequenceWaveformEditor::editableBeatLayerChanged);
        connect(m_impl->sequence, &Sequence::beatLayerAdded, this, &SequenceWaveformEditor::layerAdded);
        connect(m_impl->sequence, &Sequence::beatLayerRemoved, this, &SequenceWaveformEditor::layerRemoved);

        // A sequence with no local file (e.g. built from a VirtualDJ-live capture of
        // a streaming track) has nothing to decode - frame the time axis from
        // SongData's duration instead, if it's known yet.
        if(!t_sequence->filePath().isEmpty())
            loadAudio(t_sequence->filePath());
        else if(SongData *songData = t_sequence->songData())
            setDuration(songData->duration());

        for(auto beat : m_impl->sequence->beatLayers())
        {

            connect(beat, &BeatLayer::metadataChanged, this, &SequenceWaveformEditor::beatsMetadataUpdated);

            if(beat->isEditable())
            {
                m_impl->editableLayer = beat;
                m_impl->otherBeats = beat->beats();
            }
            else
            {
                connect(beat, &BeatLayer::beatsChanged, this, &SequenceWaveformEditor::beatsUpdated);
            }
        }
    }
}


void SequenceWaveformEditor::layerAdded(photon::BeatLayer* t_layer)
{
    connect(t_layer, &BeatLayer::beatsChanged, this, &SequenceWaveformEditor::beatsUpdated);
    connect(t_layer, &BeatLayer::metadataChanged, this, &SequenceWaveformEditor::beatsMetadataUpdated);

}

void SequenceWaveformEditor::layerRemoved(photon::BeatLayer* t_layer)
{
    disconnect(t_layer, &BeatLayer::beatsChanged, this, &SequenceWaveformEditor::beatsUpdated);
    disconnect(t_layer, &BeatLayer::metadataChanged, this, &SequenceWaveformEditor::beatsMetadataUpdated);
}

void SequenceWaveformEditor::editableBeatLayerChanged(photon::BeatLayer* t_layer)
{

    if(m_impl->editableLayer)
    {
        connect(m_impl->editableLayer, &BeatLayer::beatsChanged, this, &SequenceWaveformEditor::beatsUpdated);
    }
    m_impl->editableLayer = t_layer;
    if(t_layer)
    {
        m_impl->otherBeats = t_layer->beats();
        disconnect(t_layer, &BeatLayer::beatsChanged, this, &SequenceWaveformEditor::beatsUpdated);
    }
}

void SequenceWaveformEditor::beatsUpdated(photon::BeatLayer*beat)
{

    update();
}

void SequenceWaveformEditor::beatsMetadataUpdated(photon::BeatLayer*)
{

    update();
}

void SequenceWaveformEditor::drawFeatureOverlay(QPainter &t_painter)
{
    SongData *songData = m_impl->sequence->songData();

    // Low/mid/high (local-file analysis) are peak-tracked, so each is a real
    // band-limited waveform rather than a smoothed energy curve; stem levels
    // (VirtualDJ live capture) are VDJ's own reported values, drawn the same way.
    // Whichever set a given sequence actually has populated is what renders - a
    // sequence normally has one or the other, not both. Mirrored around the
    // vertical centre like a normal audio waveform and layered semi-transparently,
    // inspired by VirtualDJ's stacked multi-band/stem waveform.
    static const struct { QByteArray id; QColor color; } kBands[] = {
        { SongData::FeatureLow,    QColor(40, 130, 200, 130) },
        { SongData::FeatureMid,    QColor(80, 190, 230, 130) },
        { SongData::FeatureHigh,   QColor(150, 230, 255, 140) },
        { SongData::FeatureVocal,  QColor(230, 100, 190, 140) },
        { SongData::FeatureInstru, QColor(80, 190, 230, 130) },
        { SongData::FeatureBass,   QColor(40, 100, 200, 140) },
        { SongData::FeatureKick,   QColor(255, 150, 60, 150) },
        { SongData::FeatureHiHat,  QColor(200, 240, 255, 130) },
    };

    const int centreY = height() / 2;
    const int halfHeight = height() / 2;

    for(const auto &band : kBands)
    {
        const FeatureTrack *track = songData->feature(band.id);
        if(!track || track->isEmpty())
            continue;

        for(int x = 0; x < width(); ++x)
        {
            const double tLeft = xToTime(x);
            const double tRight = xToTime(x + 1);

            float lo, hi;
            if(!track->rangeBetween(tLeft, tRight, lo, hi))
                continue;

            // Peak values are non-negative; hi is this column's loudest instant in
            // the band. Mirror it around the centre for a symmetric waveform look.
            const float peak = std::clamp(hi, 0.0f, 1.0f);
            const int span = static_cast<int>(peak * halfHeight);
            t_painter.fillRect(x, centreY - span, 1, std::max(1, span * 2), band.color);
        }
    }
}

void SequenceWaveformEditor::paintEvent(QPaintEvent *t_event)
{
    WaveformWidget::paintEvent(t_event);

    if(!m_impl->sequence)
        return;

    QPainter painter{this};

    if(m_impl->sequence->songData())
        drawFeatureOverlay(painter);

    if(m_impl->dragMode == Impl::DragSelect)
    {
        auto x1 = timeToX(m_impl->selectionRange.start);
        auto x2 = timeToX(m_impl->selectionRange.end);

        painter.fillRect(x1,0,x2 - x1,height(),QColor(255,255,255,50));
    }

    // Short ticks along the bottom edge rather than full-height lines, so they read
    // as beat markers (matching a VirtualDJ-style layout) instead of competing with
    // the band overlay for the whole view's vertical space. Hit-testing in
    // mousePressEvent/mouseMoveEvent still treats the full column as clickable, so
    // editing behaviour is unchanged even though only the bottom is drawn.
    const int tickHeight = 8;
    const int tickWidth = 4;
    auto drawBeatTick = [&](double t_beat, const QColor &t_color) {
        if(!visibleRange().contains(t_beat))
            return;
        const double xd = timeToX(t_beat);
        if(!std::isfinite(xd))
            return;
        // Clamp before converting to int: timeToX() can be handed extreme beat data
        // from an external producer (e.g. a VDJ-live capture that briefly saw a
        // stale/garbage bpm reading), and QRect's checked-integer arithmetic asserts
        // fatally on overflow rather than silently wrapping or clipping.
        const int x = static_cast<int>(std::clamp(xd, -1.0e6, 1.0e6));
        painter.fillRect(x - tickWidth / 2, height() - tickHeight, tickWidth, tickHeight, t_color);
    };

    // The analysed beat grid: always drawn once a file has been analysed, distinct
    // from the red custom-cue layers below since it's derived data, not something
    // authored/edited here.
    if(SongData *songData = m_impl->sequence->songData())
    {
        for(double beat : songData->beats().beats())
            drawBeatTick(beat, QColor(255, 180, 60, 200));
    }

    for(auto beatLayer : m_impl->sequence->beatLayers())
    {
        if(!beatLayer->isVisible())
            continue;
        for(auto beat : beatLayer->beats())
            drawBeatTick(beat, Qt::red);
    }

    for(auto beat : m_impl->otherBeats)
    {
        if(m_impl->editableLayer && !m_impl->editableLayer->isVisible())
            continue;
        drawBeatTick(beat, Qt::red);
    }

    for(auto beat : m_impl->selectedBeats)
        drawBeatTick(beat, Qt::cyan);
}

void SequenceWaveformEditor::keyPressEvent(QKeyEvent *t_key)
{
    WaveformWidget::keyPressEvent(t_key);

    if(t_key->key() == Qt::Key_Delete)
    {
        qDebug() << "Delete";
        m_impl->selectedBeats.clear();
        if(m_impl->editableLayer)
        {
            m_impl->editableLayer->replaceBeats(m_impl->otherBeats);
            m_impl->editableLayer->sort();
        }
        update();
    }
    else if(t_key->key() == Qt::Key_Insert)
    {
        if(m_impl->editableLayer)
        {
            m_impl->otherBeats.append(playheadTime());
            m_impl->editableLayer->addBeats(m_impl->otherBeats);
            m_impl->editableLayer->sort();
        }
        update();
    }
}

void SequenceWaveformEditor::mousePressEvent(QMouseEvent *t_event)
{
    WaveformWidget::mousePressEvent(t_event);

    auto x = t_event->pos().x();
    auto time = xToTime(t_event->pos().x());
    m_impl->initialTime = time;

    if(t_event->modifiers() & Qt::ControlModifier)
    {


        if(m_impl->editableLayer)
            m_impl->editableLayer->insertBeat(time);
        //m_impl->otherBeats.append(time);
        qDebug() << "Add" << time;
        return;

    }

    if(!m_impl->selectedBeats.isEmpty())
    {
        for(auto it = m_impl->selectedBeats.cbegin(); it != m_impl->selectedBeats.cend(); ++it)
        {
            if(visibleRange().contains(*it))
            {
                auto beatX = timeToX(*it);
                if(x >= beatX && x <= beatX + 2)
                {
                    m_impl->dragMode = Impl::DragMove;
                    m_impl->selectedBeatsInitial = m_impl->selectedBeats;
                    return;
                }
            }
        }
    }


    if(!(t_event->modifiers() & Qt::ShiftModifier))
        m_impl->clearBeats();

    for(auto it = m_impl->otherBeats.cbegin(); it != m_impl->otherBeats.cend(); ++it)
    {
        if(visibleRange().contains(*it))
        {
            auto beatX = timeToX(*it);
            if(x >= beatX && x <= beatX + 2)
            {


                m_impl->selectedBeats.append(*it);
                m_impl->otherBeats.erase(it);
                m_impl->dragMode = Impl::DragMove;
                m_impl->selectedBeatsInitial = m_impl->selectedBeats;
                return;
            }
        }
    }



    m_impl->selectionRange.start = time;
    m_impl->selectionRange.end = time;

    if(!(t_event->modifiers() & Qt::ShiftModifier))
        m_impl->clearBeats();

    m_impl->dragMode = Impl::DragSelect;
}

void SequenceWaveformEditor::mouseMoveEvent(QMouseEvent *t_event)
{
    WaveformWidget::mouseMoveEvent(t_event);

    auto x = t_event->pos().x();
    double time = xToTime(t_event->pos().x());
    double deltaTime = time - m_impl->initialTime;

    if(t_event->buttons() & Qt::LeftButton)
    {

        if(m_impl->dragMode == Impl::DragMove)
        {
            m_impl->selectedBeats = m_impl->selectedBeatsInitial;
            for(auto &beat : m_impl->selectedBeats)
            {
                beat += deltaTime;
            }

            if(m_impl->editableLayer)
            {
                m_impl->editableLayer->replaceBeats(m_impl->selectedBeats + m_impl->otherBeats);
                m_impl->editableLayer->sort();
            }
            update();
        }
        else if(m_impl->dragMode == Impl::DragSelect)
        {
            m_impl->selectionRange.start = std::min(time, m_impl->initialTime);
            m_impl->selectionRange.end = std::max(time, m_impl->initialTime);
            update();
        }

    }
    else
    {
        if(m_impl->editableLayer)
        {
            for(auto beat : m_impl->editableLayer->beats())
            {
                auto beatX = timeToX(beat);
                if(x >= beatX && x <= beatX + 2)
                {
                    setCursor(Qt::SplitHCursor);
                    return;
                }
            }

        }
        setCursor(Qt::ArrowCursor);
    }




}

void SequenceWaveformEditor::mouseReleaseEvent(QMouseEvent *t_event)
{
    WaveformWidget::mouseReleaseEvent(t_event);

    if(m_impl->dragMode == Impl::DragSelect)
    {
        for(auto it = m_impl->otherBeats.begin(); it != m_impl->otherBeats.end();)
        {
            if(m_impl->selectionRange.contains(*it))
            {
                m_impl->selectedBeats.append(*it);
                it = m_impl->otherBeats.erase(it);
                continue;
            }
            ++it;
        }
        update();
    }


    m_impl->dragMode = Impl::DragNone;
}


} // namespace photon
