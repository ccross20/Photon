#ifndef PHOTON_DMXPATCHGRID_H
#define PHOTON_DMXPATCHGRID_H

#include <QWidget>
#include <QVector>
#include <QHash>
#include <QRect>
#include <QPoint>
#include "photon-global.h"

class QMimeData;

namespace photon {

// A 512-channel DMX patch grid for one universe. Patched fixtures are drawn
// as colored bars spanning their occupied channels (wrapping across grid rows
// if needed); dragging a bar re-patches it to a new starting channel.
// Fixtures can also be dropped in from elsewhere (e.g. the Rig panel's tree,
// which already drags scene objects via the same mime format) — dropping
// several at once patches them back-to-back starting at the drop channel.
class PHOTONCORE_EXPORT DMXPatchGrid : public QWidget
{
    Q_OBJECT
public:
    explicit DMXPatchGrid(QWidget *parent = nullptr);
    ~DMXPatchGrid();

    int universe() const;
    void setUniverse(int universe);

    void setProject(Project *project);
    // Re-reads the fixture list from the project (a full scene-tree walk) and
    // repaints. Call whenever fixtures are added/removed/re-patched elsewhere
    // — paintEvent itself never walks the scene, so dragging stays smooth.
    void refreshFixtures();

    // Synced from whatever scene objects are selected elsewhere (e.g. the Rig
    // panel's multi-selection) — replaces our whole selection with just the
    // Fixture objects among these (non-fixtures are ignored). Does not push
    // back out to the project (use this only for externally-driven sync).
    void setSelectedFixtures(const QList<SceneObject*> &objects);
    // The most recently selected fixture, if any — the primary of
    // selectedFixtures(), what gets published as the project's primary
    // selectedSceneObject.
    Fixture *selectedFixture() const;
    QVector<Fixture*> selectedFixtures() const;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dragLeaveEvent(QDragLeaveEvent *) override;
    void dropEvent(QDropEvent *) override;

private:
    // One contiguous run of cells on a single grid row. A fixture whose
    // channel range crosses a row boundary is drawn as several of these.
    struct Segment
    {
        int row;
        int colStart;
        int colEnd; // inclusive
    };

    QRectF cellRect(int channel) const;
    int channelAt(const QPointF &pos) const;
    QVector<Segment> segmentsFor(int offset, int size) const;
    QRectF segmentRect(const Segment &segment) const;
    // The sub-rect a fixture in the given lane (of maxLanes total, at this
    // segment) occupies — lets fixtures whose channel ranges overlap stack as
    // parallel, thinner bars instead of drawing directly on top of each other.
    QRectF laneRect(const Segment &segment, int lane, int maxLanes) const;
    QVector<Fixture*> fixturesInUniverse() const;
    // Assigns each fixture the lowest-numbered lane that doesn't overlap
    // anything else already in it (classic interval-partitioning), so
    // fixtures sharing channels get distinct lanes instead of colliding.
    QHash<Fixture*, int> computeLanes(const QVector<Fixture*> &fixtures, int *outMaxLanes) const;
    Fixture *fixtureAt(const QPoint &pos) const;
    QVector<Fixture*> decodeFixtures(const QMimeData *mimeData) const;
    void rebuildBackground();

    bool isSelected(Fixture *fixture) const;
    // Selection mutators for the three click modes (plain/shift/ctrl); each
    // repaints and pushes the new primary (most-recent) selection out to the
    // project so other panels (e.g. Properties) stay in sync.
    void replaceSelection(Fixture *fixture);
    void addToSelection(Fixture *fixture);
    void toggleSelection(Fixture *fixture);
    void pushPrimarySelectionToProject();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DMXPATCHGRID_H
