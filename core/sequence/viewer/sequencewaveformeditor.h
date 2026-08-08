#ifndef PHOTON_SEQUENCEWAVEFORMEDITOR_H
#define PHOTON_SEQUENCEWAVEFORMEDITOR_H
#include "gui/waveformwidget.h"
#include "photon-global.h"

class QPainter;

namespace photon {

class SequenceWaveformEditor : public WaveformWidget
{
    Q_OBJECT
public:
    SequenceWaveformEditor(Sequence *sequence = nullptr, QWidget *parent = nullptr);
    ~SequenceWaveformEditor();

    void setSequence(Sequence *);
    Sequence *sequence() const;

private slots:
    void markersUpdated(photon::CueLayer*);
    void layerAdded(photon::CueLayer*);
    void layerRemoved(photon::CueLayer*);
    void markersMetadataUpdated(photon::CueLayer*);
    void editableCueLayerChanged(photon::CueLayer*);

protected:

    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void mouseReleaseEvent(QMouseEvent *) override;
    virtual void keyPressEvent(QKeyEvent*) override;
    virtual void resizeEvent(QResizeEvent *) override;

private:
    void drawFeatureOverlay(QPainter &painter);
    void deleteSelectedMarkers();
    void updateMarkerDeleteButton();
    void positionMarkerDeleteButton();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SEQUENCEWAVEFORMEDITOR_H
