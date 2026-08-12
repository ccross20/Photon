#include <QCompleter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QRegularExpression>
#include <QStringListModel>
#include <QTimer>
#include "tageditorwidget.h"
#include "tagchip.h"
#include "tagmime.h"
#include "gui/flowlayout.h"

namespace photon {

class TagEditorWidget::Impl
{
public:
    std::function<QStringList()> get;
    std::function<void(QStringList)> set;
    std::function<QStringList()> knownTags;

    FlowLayout *layout = nullptr;
    QLineEdit *addEdit = nullptr;
    QCompleter *completer = nullptr;
    QVector<TagChip*> chips;
};

TagEditorWidget::TagEditorWidget(std::function<QStringList()> t_get,
                                 std::function<void(QStringList)> t_set,
                                 std::function<QStringList()> t_knownTags,
                                 QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    m_impl->get = t_get;
    m_impl->set = t_set;
    m_impl->knownTags = t_knownTags;

    setAcceptDrops(true);

    m_impl->layout = new FlowLayout(this, 4, 4, 4);

    m_impl->addEdit = new QLineEdit;
    m_impl->addEdit->setPlaceholderText("+ tag");
    m_impl->addEdit->setFixedWidth(80);

    m_impl->completer = new QCompleter(this);
    m_impl->completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_impl->completer->setFilterMode(Qt::MatchContains);
    m_impl->completer->setCompletionMode(QCompleter::PopupCompletion);
    m_impl->addEdit->setCompleter(m_impl->completer);

    m_impl->layout->addWidget(m_impl->addEdit);

    connect(m_impl->addEdit, &QLineEdit::editingFinished, this, &TagEditorWidget::commitPendingText);
    connect(m_impl->completer, QOverload<const QString&>::of(&QCompleter::activated),
            this, &TagEditorWidget::completionActivated);

    refresh();
}

TagEditorWidget::~TagEditorWidget()
{
    delete m_impl;
}

void TagEditorWidget::refresh()
{
    for(auto *chip : m_impl->chips)
    {
        m_impl->layout->removeWidget(chip);
        chip->deleteLater();
    }
    m_impl->chips.clear();

    // The add field is always the trailing item - pull it out, re-add the
    // chips, then put it back at the end.
    m_impl->layout->removeWidget(m_impl->addEdit);

    QStringList tags = m_impl->get ? m_impl->get() : QStringList();
    for(const QString &tag : tags)
    {
        auto *chip = new TagChip(tag, this);
        connect(chip, &TagChip::removeRequested, this, &TagEditorWidget::chipRemoveRequested);
        m_impl->layout->addWidget(chip);
        m_impl->chips.append(chip);
    }

    m_impl->layout->addWidget(m_impl->addEdit);

    rebuildCompleter();
}

void TagEditorWidget::rebuildCompleter()
{
    if(!m_impl->knownTags)
        return;

    QStringList known = m_impl->knownTags();
    const QStringList current = m_impl->get ? m_impl->get() : QStringList();

    // No point suggesting a tag that's already applied here.
    for(const QString &tag : current)
        known.removeAll(tag);

    m_impl->completer->setModel(new QStringListModel(known, m_impl->completer));
}

void TagEditorWidget::addTags(const QStringList &t_tags)
{
    if(t_tags.isEmpty() || !m_impl->get || !m_impl->set)
        return;

    QStringList combined = m_impl->get();
    bool changed = false;

    for(const QString &raw : t_tags)
    {
        const QString tag = raw.trimmed().toLower();
        if(tag.isEmpty() || combined.contains(tag))
            continue;
        combined.append(tag);
        changed = true;
    }

    if(changed)
    {
        m_impl->set(combined);
        refresh();
    }
}

void TagEditorWidget::chipRemoveRequested(QString t_tag)
{
    if(!m_impl->get || !m_impl->set)
        return;

    QStringList remaining = m_impl->get();
    remaining.removeAll(t_tag);
    m_impl->set(remaining);
    refresh();
}

void TagEditorWidget::commitPendingText()
{
    const QString text = m_impl->addEdit->text();
    if(text.trimmed().isEmpty())
        return;

    // A paste can carry several tags at once - split on the separators every
    // prior free-text tag field in the app used (comma or whitespace).
    const QStringList typed = text.split(QRegularExpression("[,\\s]+"), Qt::SkipEmptyParts);
    m_impl->addEdit->clear();
    addTags(typed);
}

void TagEditorWidget::completionActivated(const QString &t_text)
{
    addTags({t_text});

    // QLineEdit wires itself directly to the completer's activated() signal
    // (to fill the field with the chosen completion) when setCompleter() is
    // called, before our own connection to the same signal - so a plain
    // clear() here can still be overwritten by that once the popup finishes
    // closing. Deferring to the next event-loop turn guarantees this runs
    // after any of that settles.
    QTimer::singleShot(0, m_impl->addEdit, &QLineEdit::clear);
}

void TagEditorWidget::dragEnterEvent(QDragEnterEvent *t_event)
{
    if(t_event->mimeData()->hasFormat(TagMimeType))
        t_event->acceptProposedAction();
}

void TagEditorWidget::dropEvent(QDropEvent *t_event)
{
    const QStringList tags = decodeTagMime(t_event->mimeData());
    if(tags.isEmpty())
        return;

    addTags(tags);
    t_event->acceptProposedAction();
}

} // namespace photon
