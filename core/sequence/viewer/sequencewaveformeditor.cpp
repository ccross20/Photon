#include <QPainter>
#include <QMouseEvent>
#include "sequencewaveformeditor.h"
#include "sequence/sequence.h"
#include "sequence/beatlayer.h"

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

        loadAudio(t_sequence->filePath());

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

void SequenceWaveformEditor::paintEvent(QPaintEvent *t_event)
{
    WaveformWidget::paintEvent(t_event);

    if(!m_impl->sequence)
        return;

    QPainter painter{this};

    if(m_impl->dragMode == Impl::DragSelect)
    {
        auto x1 = timeToX(m_impl->selectionRange.start);
        auto x2 = timeToX(m_impl->selectionRange.end);

        painter.fillRect(x1,0,x2 - x1,height(),QColor(255,255,255,50));
    }

    for(auto beatLayer : m_impl->sequence->beatLayers())
    {
        if(!beatLayer->isVisible())
            continue;
        for(auto beat : beatLayer->beats())
        {
            if(visibleRange().contains(beat))
            {
                auto x = timeToX(beat);
                painter.fillRect(x,0,2,height(),Qt::red);
            }

        }
    }

    for(auto beat : m_impl->otherBeats)
    {
        if(m_impl->editableLayer && !m_impl->editableLayer->isVisible())
            continue;
        if(visibleRange().contains(beat))
        {
            auto x = timeToX(beat);
            painter.fillRect(x,0,2,height(),Qt::red);
        }
    }

    for(auto beat : m_impl->selectedBeats)
    {
        if(visibleRange().contains(beat))
        {
            auto x = timeToX(beat);
            painter.fillRect(x,0,2,height(),Qt::cyan);
        }
    }
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
