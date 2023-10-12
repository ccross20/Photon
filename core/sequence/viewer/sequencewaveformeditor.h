#ifndef PHOTON_SEQUENCEWAVEFORMEDITOR_H
#define PHOTON_SEQUENCEWAVEFORMEDITOR_H
#include "gui/waveformwidget.h"
#include "photon-global.h"

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
    void beatsUpdated(photon::BeatLayer*);
    void layerAdded(photon::BeatLayer*);
    void layerRemoved(photon::BeatLayer*);
    void beatsMetadataUpdated(photon::BeatLayer*);
    void editableBeatLayerChanged(photon::BeatLayer*);

protected:

    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void mouseMoveEvent(QMouseEvent *) override;
    virtual void mouseReleaseEvent(QMouseEvent *) override;
    virtual void keyPressEvent(QKeyEvent*) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SEQUENCEWAVEFORMEDITOR_H
