#include <QMessageBox>
#include "sequencecollection.h"
#include "sequence.h"
#include "gui/panel/sequencepanel.h"
#include "photoncore.h"
#include "project/project.h"
#include "third-party/advanced-docking/DockWidget.h"

namespace photon {

class SequenceCollection::Impl
{
public:
    Impl(SequenceCollection *);

    QHash<Sequence*,SequencePanel*> panels;
    QVector<Sequence *> sequences;
    SequenceCollection *facade;
    Sequence *activeSequence = nullptr;
    SequencePanel *activePanel = nullptr;
    bool ownsSequences = true;
};

SequenceCollection::Impl::Impl(SequenceCollection *t_facade):facade(t_facade)
{

}

SequenceCollection::SequenceCollection(bool t_ownsSequences, QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{
    m_impl->ownsSequences = t_ownsSequences;
}

SequenceCollection::~SequenceCollection()
{
    if(m_impl->ownsSequences)
        for(auto sequence : m_impl->sequences)
            delete sequence;
    delete m_impl;
}

Sequence *SequenceCollection::activeSequence() const
{
    return m_impl->activeSequence;
}

void SequenceCollection::setActiveSequencePanel(SequencePanel *panel)
{
    m_impl->activePanel = panel;

    if(panel)
        photonApp->gui()->bringPanelToFront(panel);
}

SequencePanel *SequenceCollection::activeSequencePanel() const
{
    return m_impl->activePanel;
}

SequencePanel *SequenceCollection::panelFor(Sequence *t_sequence) const
{
    return m_impl->panels.value(t_sequence, nullptr);
}

void SequenceCollection::editSequence(Sequence *t_sequence)
{
    m_impl->activeSequence = t_sequence;
    if(m_impl->panels.contains(t_sequence))
    {
        setActiveSequencePanel(m_impl->panels.value(t_sequence));
    }
    else
    {
        SequencePanel *sequencePanel = static_cast<SequencePanel*>(photonApp->gui()->createDockedPanel("photon.sequence", GuiManager::CenterDockWidgetArea, true));
        sequencePanel->setSequence(t_sequence);
        sequencePanel->setName(t_sequence->name());
        m_impl->panels.insert(t_sequence, sequencePanel);
        connect(sequencePanel, &QObject::destroyed,this, &SequenceCollection::panelDestroyed);

        // A Song Library sequence has its own file to lose changes to -
        // intercept the tab's close button and ask before it actually
        // closes. An internal (project-embedded) sequence has no separate
        // file of its own to save, so it closes without asking (its content
        // is only ever saved along with the project).
        if(auto *dockWidget = sequencePanel->dockWidget())
        {
            dockWidget->setFeature(ads::CDockWidget::CustomCloseHandling, true);
            connect(dockWidget, &ads::CDockWidget::closeRequested, this, [this, dockWidget, t_sequence](){
                if(t_sequence->isLibrarySequence())
                {
                    const auto choice = QMessageBox::question(dockWidget, "Save Sequence",
                        QString("Save changes to \"%1\" before closing?").arg(t_sequence->name()),
                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
                    if(choice == QMessageBox::Cancel)
                        return;
                    if(choice == QMessageBox::Save)
                        t_sequence->save(t_sequence->filePath());

                    // Unlike an internal (project-embedded) sequence, which
                    // stays tracked by the project regardless of any open
                    // editor, a Song Library sequence has no other reason to
                    // stay resident once its tab is closed - leaving it
                    // loaded would just accumulate silently and get asked
                    // about again at shutdown for a tab that's long gone.
                    // removeSequence() closes this same tab itself.
                    removeSequence(t_sequence);
                    return;
                }
                dockWidget->closeDockWidget();
            });
        }

        setActiveSequencePanel(sequencePanel);
    }
}

void SequenceCollection::panelDestroyed(QObject *t_object)
{
    // Closing an internal (project-embedded) sequence's editor doesn't unload
    // it - it stays tracked by the project either way, so this just forgets
    // the (now-dead) panel, and editSequence() opens a fresh one next time
    // it's edited. A Song Library sequence's own tab-close handler (see
    // editSequence() above) already unloads it via removeSequence() before
    // this ever runs for one.
    for(auto it = m_impl->panels.constBegin(); it != m_impl->panels.constEnd(); ++it)
    {
        if(it.value() == static_cast<SequencePanel*>(t_object))
        {
            m_impl->panels.remove(it.key());
            break;
        }
    }

}

void SequenceCollection::clear()
{
    if(m_impl->ownsSequences)
    {
        // Same ordering concern as removeSequence(): close (deferred) any
        // still-open editor before the sequence itself is deferred-deleted,
        // so a widget that stays alive/paintable until its own teardown runs
        // never ends up pointing at freed memory in the meantime.
        for(auto sequence : m_impl->sequences)
        {
            if(SequencePanel *panel = m_impl->panels.value(sequence))
            {
                if(auto *dockWidget = panel->dockWidget())
                    dockWidget->closeDockWidget();
            }
            sequence->deleteLater();
        }
        m_impl->panels.clear();
    }

    m_impl->activeSequence = nullptr;
    m_impl->activePanel = nullptr;
    m_impl->sequences.clear();
}

const QVector<Sequence*> &SequenceCollection::sequences() const
{
    return m_impl->sequences;
}

int SequenceCollection::sequenceCount() const
{
    return m_impl->sequences.length();
}

Sequence *SequenceCollection::sequenceAtIndex(uint t_index) const
{
    return m_impl->sequences.at(t_index);
}

void SequenceCollection::addSequence(photon::Sequence *t_sequence)
{
    if(m_impl->sequences.contains(t_sequence))
        return;
    emit sequenceWillBeAdded(t_sequence, m_impl->sequences.length());
    m_impl->sequences.append(t_sequence);
    emit sequenceWasAdded(t_sequence, m_impl->sequences.length()-1);
}

void SequenceCollection::removeSequence(photon::Sequence *t_sequence)
{
    if(!m_impl->sequences.contains(t_sequence))
        return;

    int index = 0;
    for(auto seq : m_impl->sequences)
    {
        if(seq == t_sequence)
            break;
        ++index;
    }

    // Close (deferred - ads::CDockWidget::closeDockWidget() only schedules
    // deletion via deleteLater(), it doesn't delete synchronously) any editor
    // still open on this sequence. This must be requested BEFORE the
    // sequence's own deleteLater() below so the widget tree's teardown is
    // queued first - it stays fully alive/paintable until then, so deleting
    // the sequence first would leave it pointing at freed memory in the
    // meantime.
    if(SequencePanel *panel = m_impl->panels.value(t_sequence))
    {
        if(auto *dockWidget = panel->dockWidget())
            dockWidget->closeDockWidget();
    }
    m_impl->panels.remove(t_sequence);

    // Neither can be left pointing at a sequence that's about to be deleted
    // below (an owning collection) or that's simply no longer tracked (a
    // non-owning one, e.g. Project::sequences()).
    if(m_impl->activeSequence == t_sequence)
    {
        m_impl->activeSequence = nullptr;
        m_impl->activePanel = nullptr;
    }
    if(Project *project = photonApp->project())
    {
        QList<ProjectResource*> selection = project->selectedResources();
        if(selection.removeOne(static_cast<ProjectResource*>(t_sequence)))
            project->setSelectedResources(selection);
    }

    emit sequenceWillBeRemoved(t_sequence, index);
    m_impl->sequences.removeOne(t_sequence);
    emit sequenceWasRemoved(t_sequence, index);

    // Only the owning collection (PhotonCore's app-wide one) actually frees
    // it - see the ownsSequences constructor parameter. A non-owning one
    // (Project::sequences()) is just a membership list over the same
    // objects and must not double-delete them. Deferred (not a synchronous
    // delete) for the same reason as the panel close above - anything that
    // queued its own deferred cleanup against this sequence just above gets
    // to run first.
    if(m_impl->ownsSequences)
        t_sequence->deleteLater();
}

} // namespace photon
