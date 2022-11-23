#include "sequencepanel.h"
#include "sequence/sequence.h"
#include "photoncore.h"
#include "sequence/viewer/sequencewidget.h"

namespace photon {

class SequencePanel::Impl
{
public:
    SequenceWidget *sequenceViewer;
};

SequencePanel::SequencePanel() : Panel("sequence"),m_impl(new Impl)
{


    m_impl->sequenceViewer = new SequenceWidget;

    setPanelWidget(m_impl->sequenceViewer);

}

SequencePanel::~SequencePanel()
{
    if(photonApp->activeSequencePanel() == this)
        photonApp->setActiveSequencePanel(nullptr);
    delete m_impl;
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

void SequencePanel::didActivate()
{
    Panel::didActivate();
}

void SequencePanel::didDeactivate()
{
    Panel::didDeactivate();


}

} // namespace photon
