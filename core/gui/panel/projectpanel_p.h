#ifndef PROJECTPANEL_P_H
#define PROJECTPANEL_P_H

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <functional>
#include "projectpanel.h"
#include "project/projectmodel.h"

namespace photon {

// Paints a resource's tags as colored chips instead of joined text, in the
// tree's Tags column - the same paintTagChip() a TagChip widget uses, so a
// chip looks identical whether it's shown here or in a properties editor.
// resolver maps a (proxy) QModelIndex to the ProjectResource it represents,
// reusing ProjectPanel::Impl::resourceAt() rather than duplicating the
// proxy-to-source lookup.
class ProjectTagDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ProjectTagDelegate(std::function<ProjectResource*(const QModelIndex&)> resolver, QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // The tag (if any) at pos (viewport coordinates) inside index's cell, or
    // empty if none - including when pos falls on the "+N" overflow marker
    // rather than a real chip. Shared geometry with paint() (via
    // layoutChips()) so hit-testing can't drift from what's actually drawn.
    QString chipAt(QTreeView *view, const QModelIndex &index, const QPoint &pos) const;

private:
    QVector<QPair<QString, QRect>> layoutChips(const QRect &rect, const QStringList &tags, const QFontMetrics &metrics) const;

    std::function<ProjectResource*(const QModelIndex&)> m_resolver;
};

// Matches the search text against a row's name and its tags, so typing a tag
// finds every resource carrying it regardless of type. Recursive filtering is
// on, so a matching child keeps its folder (and, in the Rig, its ancestors)
// visible.
class ProjectFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ProjectFilterModel(QObject *parent = nullptr);

    void setSearchText(const QString &);
    // Restrict to certain resource type ids; empty means no restriction.
    void setTypeFilter(const QSet<QByteArray> &);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_search;
    QSet<QByteArray> m_types;
};

class ProjectPanel::Impl
{
public:
    QVBoxLayout *layout = nullptr;
    QLineEdit *searchEdit = nullptr;
    QPushButton *typeButton = nullptr;
    QTreeView *treeView = nullptr;
    QPushButton *addButton = nullptr;
    QPushButton *removeButton = nullptr;
    QPushButton *duplicateButton = nullptr;

    ProjectModel *model = nullptr;
    ProjectFilterModel *proxy = nullptr;
    ProjectTagDelegate *tagDelegate = nullptr;

    // Guards the project -> view selection sync so it doesn't echo back out.
    bool syncingFromProject = false;

    QSet<QByteArray> hiddenTypes;

    // A tag chip pressed in the Tags column, waiting to see if the mouse
    // moves far enough to become a drag. Cleared on release or once the drag
    // actually starts.
    QString pendingDragTag;
    QPoint pendingDragPos;

    // True from a left mouse press on the tree until we know whether the
    // gesture is a click or a drag - resolved either at release (it was a
    // click) or as soon as the mouse crosses the drag-start distance (it's a
    // drag). While true, a selection change is deferred rather than
    // published - see selectionChanged().
    bool resolvingClickOrDrag = false;
    // A selection change arrived while resolvingClickOrDrag was true.
    // Published once resolved as a click (a click that changes selection
    // immediately on press won't get another selectionChanged at release to
    // publish from); discarded if resolved as a drag instead - dragging an
    // unselected row away shouldn't flash the Properties panel to it.
    bool pendingSelectionPublish = false;

    ProjectResource *resourceAt(const QModelIndex &proxyIndex) const;
    QList<ProjectResource*> selectedResources() const;
};

} // namespace photon

#endif // PROJECTPANEL_P_H
