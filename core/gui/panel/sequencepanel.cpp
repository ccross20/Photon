#include <QKeyEvent>
#include "sequencepanel.h"
#include "sequence/sequence.h"
#include "photoncore.h"
#include "sequence/viewer/sequencewidget.h"
#include "sequence/sequencecollection.h"

namespace photon {

class SequencePanel::Impl
{
public:
    SequenceWidget *sequenceViewer;
};

SequencePanel::SequencePanel() : Panel("photon.sequence"),m_impl(new Impl)
{

    setName("Sequence");
    m_impl->sequenceViewer = new SequenceWidget;

    setPanelWidget(m_impl->sequenceViewer);

}

SequencePanel::~SequencePanel()
{
    if(photonApp->sequences()->activeSequencePanel() == this)
        photonApp->sequences()->setActiveSequencePanel(nullptr);
    delete m_impl;
}

bool SequencePanel::isPlaying() const
{
    return m_impl->sequenceViewer->isPlaying();
}

DMXMatrix SequencePanel::getDMX()
{
    return m_impl->sequenceViewer->getDMX();
}

void SequencePanel::processPreview(ProcessContext &context)
{

    m_impl->sequenceViewer->processPreview(context);
}


void SequencePanel::setSequence(Sequence *t_sequence)
{
    m_impl->sequenceViewer->setSequence(t_sequence);
}

Sequence *SequencePanel::sequence() const
{
    return m_impl->sequenceViewer->sequence();
}

bool SequencePanel::panelKeyPress(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
    {
        m_impl->sequenceViewer->togglePlay(!m_impl->sequenceViewer->isPlaying());
        return true;
    }
    return false;
}

bool SequencePanel::panelKeyRelease(QKeyEvent *event)
{
    return false;
}

void SequencePanel::didActivate()
{
    Panel::didActivate();
}

void SequencePanel::didDeactivate()
{
    Panel::didDeactivate();


}

} // namespace photon
