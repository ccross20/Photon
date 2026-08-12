#ifndef PHOTON_PIXELSOURCELAYOUT_H
#define PHOTON_PIXELSOURCELAYOUT_H

#include "photon-global.h"
#include "pixel/canvas.h"

namespace photon {

class PHOTONCORE_EXPORT PixelSourceLayout : public QObject
{
    Q_OBJECT
public:
    PixelSourceLayout(PixelSource *source = nullptr);
    ~PixelSourceLayout();

    PixelSource *source() const;
    void setSource(PixelSource *);

    // Per-pixel absolute canvas-space positions, index-parallel to the
    // source's pixel slots - the source only supplies how many pixels it has
    // (pixelCount()); where each one sits is owned entirely here (there's no
    // separate whole-fixture placement transform - Move/Scale/Rotate are
    // one-shot commands applied directly to a pixel selection, not persisted
    // per-source state). Lazily reconciled against source()->pixelCount()
    // (grown/seeded via PixelArrange::linear(), or truncated) on first
    // access after a resize.
    int pixelCount() const;
    const QVector<QPointF> &pixelPositions() const;
    QPointF pixelPosition(int index) const;

    // Clamped to pixelBounds() - the single, path-independent enforcement
    // point for every write (interactive drag commit, Arrange/Move/Scale/
    // Rotate commands), so a pixel can never end up somewhere it can't be
    // dragged back from.
    void setPixelPosition(int index, const QPointF &);

    // The normalized edit-area rect every pixel position is clamped to.
    static QRectF pixelBounds();

    // Appends this source's canvas-space sample points, in the same order
    // process() consumes colours - wraps source()->collectSampleUVs() with
    // this layout's own pixelPositions().
    void collectSampleUVs(QVector<QPointF> &out) const;

    void process(ProcessContext &, double blend = 1.0) const;

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:
    // Emitted by setPixelPosition() - a drag commits its own item's position
    // directly (it's already where the mouse left it), but a programmatic
    // write (e.g. an Arrange command) needs this so the editor's graphics
    // items know to re-read the model and redraw at their new spot.
    void pixelPositionsChanged();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELSOURCELAYOUT_H
