#include <QPainter>
#include <QMouseEvent>
#include <QToolButton>
#include <algorithm>
#include <cmath>
#include "sequencewaveformeditor.h"
#include "sequence/sequence.h"
#include "sequence/cuelayer.h"
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
    void clearMarkers();

    Sequence *sequence;
    QVector<float> otherMarkers;
    QVector<float> selectedMarkers;
    QVector<float> selectedMarkersInitial;
    CueLayer *editableLayer = nullptr;
    DragMode dragMode = DragNone;
    double initialTime = 0;
    Range selectionRange;
    QToolButton *deleteButton = nullptr;
};

void SequenceWaveformEditor::Impl::clearMarkers()
{
    selectedMarkers.clear();
    otherMarkers.clear();
    editableLayer = sequence->editableCueLayer();
    if(editableLayer)
        otherMarkers = editableLayer->markers();
}

SequenceWaveformEditor::SequenceWaveformEditor(Sequence *t_sequence, QWidget *t_parent): WaveformWidget(t_parent),m_impl(new Impl)
{
    m_impl->sequence = t_sequence;
    setFocusPolicy(Qt::StrongFocus);

    // The layered low/mid/high band overlay (see drawFeatureOverlay) replaces the
    // raw audio waveform as this view's primary visual.
    setShowWaveform(false);

    // Trashcan delete button, overlaid in the corner of the view - only shown while
    // markers are selected (see updateMarkerDeleteButton()), positioned on resize
    // (see positionMarkerDeleteButton()).
    m_impl->deleteButton = new QToolButton(this);
    m_impl->deleteButton->setText(QStringLiteral(u"\xD83D\xDDD1\xFE0F")); // U+1F5D1 wastebasket
    m_impl->deleteButton->setToolTip("Delete selected markers");
    m_impl->deleteButton->setAutoRaise(true);
    m_impl->deleteButton->setCursor(Qt::PointingHandCursor);
    m_impl->deleteButton->setVisible(false);
    connect(m_impl->deleteButton, &QToolButton::clicked, this, &SequenceWaveformEditor::deleteSelectedMarkers);
    positionMarkerDeleteButton();
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
        connect(m_impl->sequence, &Sequence::editableCueLayerChanged, this, &SequenceWaveformEditor::editableCueLayerChanged);
        connect(m_impl->sequence, &Sequence::cueLayerAdded, this, &SequenceWaveformEditor::layerAdded);
        connect(m_impl->sequence, &Sequence::cueLayerRemoved, this, &SequenceWaveformEditor::layerRemoved);

        // A sequence with no local file (e.g. built from a VirtualDJ-live capture of
        // a streaming track) has nothing to decode - frame the time axis from
        // SongData's duration instead, if it's known yet.
        if(!t_sequence->filePath().isEmpty())
            loadAudio(t_sequence->filePath());
        else if(SongData *songData = t_sequence->songData())
            setDuration(songData->duration());

        for(auto layer : m_impl->sequence->cueLayers())
        {

            connect(layer, &CueLayer::metadataChanged, this, &SequenceWaveformEditor::markersMetadataUpdated);

            if(layer->isEditable())
            {
                m_impl->editableLayer = layer;
                m_impl->otherMarkers = layer->markers();
            }
            else
            {
                connect(layer, &CueLayer::markersChanged, this, &SequenceWaveformEditor::markersUpdated);
            }
        }
    }
}


void SequenceWaveformEditor::layerAdded(photon::CueLayer* t_layer)
{
    connect(t_layer, &CueLayer::markersChanged, this, &SequenceWaveformEditor::markersUpdated);
    connect(t_layer, &CueLayer::metadataChanged, this, &SequenceWaveformEditor::markersMetadataUpdated);

}

void SequenceWaveformEditor::layerRemoved(photon::CueLayer* t_layer)
{
    disconnect(t_layer, &CueLayer::markersChanged, this, &SequenceWaveformEditor::markersUpdated);
    disconnect(t_layer, &CueLayer::metadataChanged, this, &SequenceWaveformEditor::markersMetadataUpdated);
}

void SequenceWaveformEditor::editableCueLayerChanged(photon::CueLayer* t_layer)
{

    if(m_impl->editableLayer)
    {
        connect(m_impl->editableLayer, &CueLayer::markersChanged, this, &SequenceWaveformEditor::markersUpdated);
    }
    m_impl->editableLayer = t_layer;
    if(t_layer)
    {
        m_impl->otherMarkers = t_layer->markers();
        disconnect(t_layer, &CueLayer::markersChanged, this, &SequenceWaveformEditor::markersUpdated);
    }
}

void SequenceWaveformEditor::markersUpdated(photon::CueLayer*)
{

    update();
}

void SequenceWaveformEditor::markersMetadataUpdated(photon::CueLayer*)
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

    // The analysed beat grid, as an alternating background tint rather than a row
    // of ticks: every other beat-to-beat interval gets a subtle highlight, the
    // ones in between stay the plain background, so the rhythm reads as a pulse
    // behind the waveform instead of competing with it for the view's height.
    // Drawn before the feature overlay (whose bands are largely semi-transparent)
    // so it shows through rather than getting hidden underneath.
    if(SongData *songData = m_impl->sequence->songData())
    {
        const QVector<double> &beats = songData->beats().beats();
        static const QColor kBeatTint(255, 255, 255, 18);
        for(int i = 0; i + 1 < beats.size(); i += 2)
        {
            const double startTime = beats[i];
            const double endTime = beats[i + 1];
            if(endTime < visibleRange().start || startTime > visibleRange().end)
                continue;

            const double xd1 = timeToX(startTime);
            const double xd2 = timeToX(endTime);
            if(!std::isfinite(xd1) || !std::isfinite(xd2))
                continue;

            // Clamp before converting to int - see the note on drawTick() below.
            const int x1 = static_cast<int>(std::clamp(xd1, -1.0e6, 1.0e6));
            const int x2 = static_cast<int>(std::clamp(xd2, -1.0e6, 1.0e6));
            painter.fillRect(x1, 0, x2 - x1, height(), kBeatTint);
        }
    }

    if(m_impl->sequence->songData())
        drawFeatureOverlay(painter);

    if(m_impl->dragMode == Impl::DragSelect)
    {
        auto x1 = timeToX(m_impl->selectionRange.start);
        auto x2 = timeToX(m_impl->selectionRange.end);

        painter.fillRect(x1,0,x2 - x1,height(),QColor(255,255,255,50));
    }

    // Short ticks along the top edge for cue-layer markers, rather than
    // full-height lines, so they don't compete with the band overlay for the
    // whole view's vertical space. Hit-testing in mousePressEvent/mouseMoveEvent
    // still treats the full column as clickable, so editing behaviour is
    // unchanged even though only a thin strip is drawn.
    const int tickHeight = 8;
    const int tickWidth = 4;
    auto drawTick = [&](double t_time, const QColor &t_color) {
        if(!visibleRange().contains(t_time))
            return;
        const double xd = timeToX(t_time);
        if(!std::isfinite(xd))
            return;
        // Clamp before converting to int: timeToX() can be handed extreme beat data
        // from an external producer (e.g. a VDJ-live capture that briefly saw a
        // stale/garbage bpm reading), and QRect's checked-integer arithmetic asserts
        // fatally on overflow rather than silently wrapping or clipping.
        const int x = static_cast<int>(std::clamp(xd, -1.0e6, 1.0e6));
        painter.fillRect(x - tickWidth / 2, 0, tickWidth, tickHeight, t_color);
    };

    for(auto cueLayer : m_impl->sequence->cueLayers())
    {
        if(!cueLayer->isVisible())
            continue;
        for(auto marker : cueLayer->markers())
            drawTick(marker, cueLayer->color());
    }

    for(auto marker : m_impl->otherMarkers)
    {
        if(m_impl->editableLayer && !m_impl->editableLayer->isVisible())
            continue;
        drawTick(marker, m_impl->editableLayer ? m_impl->editableLayer->color() : Qt::red);
    }

    for(auto marker : m_impl->selectedMarkers)
        drawTick(marker, Qt::cyan);
}

void SequenceWaveformEditor::keyPressEvent(QKeyEvent *t_key)
{
    WaveformWidget::keyPressEvent(t_key);

    if(t_key->key() == Qt::Key_Delete)
    {
        deleteSelectedMarkers();
    }
    else if(t_key->key() == Qt::Key_Insert)
    {
        if(m_impl->editableLayer)
        {
            m_impl->otherMarkers.append(playheadTime());
            m_impl->editableLayer->addMarkers(m_impl->otherMarkers);
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
        {
            m_impl->editableLayer->insertMarker(time);
            // Keep the hit-testing cache in sync too, or this marker won't be
            // selectable/draggable until something else (e.g. clearMarkers())
            // happens to refresh it from the layer.
            m_impl->otherMarkers.append(time);
        }
        qDebug() << "Add" << time;
        update();
        return;

    }

    if(!m_impl->selectedMarkers.isEmpty())
    {
        for(auto it = m_impl->selectedMarkers.cbegin(); it != m_impl->selectedMarkers.cend(); ++it)
        {
            if(visibleRange().contains(*it))
            {
                auto markerX = timeToX(*it);
                if(x >= markerX && x <= markerX + 2)
                {
                    m_impl->dragMode = Impl::DragMove;
                    m_impl->selectedMarkersInitial = m_impl->selectedMarkers;
                    return;
                }
            }
        }
    }


    if(!(t_event->modifiers() & Qt::ShiftModifier))
    {
        m_impl->clearMarkers();
        updateMarkerDeleteButton();
    }

    for(auto it = m_impl->otherMarkers.cbegin(); it != m_impl->otherMarkers.cend(); ++it)
    {
        if(visibleRange().contains(*it))
        {
            auto markerX = timeToX(*it);
            if(x >= markerX && x <= markerX + 2)
            {


                m_impl->selectedMarkers.append(*it);
                m_impl->otherMarkers.erase(it);
                m_impl->dragMode = Impl::DragMove;
                m_impl->selectedMarkersInitial = m_impl->selectedMarkers;
                updateMarkerDeleteButton();
                return;
            }
        }
    }



    m_impl->selectionRange.start = time;
    m_impl->selectionRange.end = time;

    if(!(t_event->modifiers() & Qt::ShiftModifier))
    {
        m_impl->clearMarkers();
        updateMarkerDeleteButton();
    }

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
            m_impl->selectedMarkers = m_impl->selectedMarkersInitial;
            for(auto &marker : m_impl->selectedMarkers)
            {
                marker += deltaTime;
            }

            if(m_impl->editableLayer)
            {
                m_impl->editableLayer->replaceMarkers(m_impl->selectedMarkers + m_impl->otherMarkers);
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
            for(auto marker : m_impl->editableLayer->markers())
            {
                auto markerX = timeToX(marker);
                if(x >= markerX && x <= markerX + 2)
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
        for(auto it = m_impl->otherMarkers.begin(); it != m_impl->otherMarkers.end();)
        {
            if(m_impl->selectionRange.contains(*it))
            {
                m_impl->selectedMarkers.append(*it);
                it = m_impl->otherMarkers.erase(it);
                continue;
            }
            ++it;
        }
        updateMarkerDeleteButton();
        update();
    }


    m_impl->dragMode = Impl::DragNone;
}

void SequenceWaveformEditor::deleteSelectedMarkers()
{
    if(m_impl->selectedMarkers.isEmpty())
        return;

    m_impl->selectedMarkers.clear();
    if(m_impl->editableLayer)
    {
        m_impl->editableLayer->replaceMarkers(m_impl->otherMarkers);
        m_impl->editableLayer->sort();
    }
    updateMarkerDeleteButton();
    update();
}

void SequenceWaveformEditor::updateMarkerDeleteButton()
{
    m_impl->deleteButton->setVisible(!m_impl->selectedMarkers.isEmpty());
}

void SequenceWaveformEditor::positionMarkerDeleteButton()
{
    const int margin = 4;
    const QSize hint = m_impl->deleteButton->sizeHint();
    m_impl->deleteButton->move(width() - hint.width() - margin, margin);
}

void SequenceWaveformEditor::resizeEvent(QResizeEvent *t_event)
{
    WaveformWidget::resizeEvent(t_event);
    positionMarkerDeleteButton();
}


} // namespace photon
