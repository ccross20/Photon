#ifndef PHOTON_SEQUENCEWIDGET_H
#define PHOTON_SEQUENCEWIDGET_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SequenceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceWidget(QWidget *parent = nullptr);
    ~SequenceWidget();

    void setSequence(Sequence *);
    Sequence *sequence() const;

    void processPreview(ProcessContext &context);
    bool isPlaying() const;

    DMXMatrix getDMX();

public slots:
    void togglePlay(bool);
    void rewind();
    // Frames the whole song (SongData duration or decoded audio length) in the
    // current viewport width. With no song data at all, there's nothing to
    // fit, so this just pans back to time 0 at the current zoom instead.
    void zoomToFitSong();
    void setScale(double);
    void setScalePoint(QPointF);
    void setOffset(double);
    void gotoTime(double);

private slots:
    void tick();
    void waveformRangeChanged(double start, double end);
    void detailsSplitterMoved(int, int);
    void editorSplitterMoved(int, int);
    void horizontalSplitterMoved(int, int);
    void selectionChanged();
    void positionChanged(qint64);
    void editLayer(photon::Layer *);
    void selectEffect(photon::ChannelEffect *);
    void selectClipGraph(photon::Clip *);
    void clearEditor();
    void showDefaultEditor();
    void toggleVdjSync(bool);

signals:
    void addedToSelection(photon::Clip*);
    void removedFromSelection(photon::Clip*);

protected:
    void showEvent(QShowEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    bool eventFilter(QObject *, QEvent *) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SEQUENCEWIDGET_H
