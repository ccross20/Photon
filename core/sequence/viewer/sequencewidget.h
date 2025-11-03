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
    void setScale(double);
    void setScalePoint(QPointF);
    void setOffset(double);
    void gotoTime(double);
    void pickFile();

private slots:
    void tick();
    void detailsSplitterMoved(int, int);
    void editorSplitterMoved(int, int);
    void horizontalSplitterMoved(int, int);
    void selectionChanged();
    void positionChanged(qint64);
    void editLayer(photon::Layer *);
    void selectEffect(photon::ChannelEffect *);
    void selectMask(photon::MaskEffect *);
    void selectClipParameter(photon::Clip *);
    void selectFalloff(photon::FalloffEffect *);
    void selectClipEffect(photon::BaseEffect *);
    void selectState(photon::State *);
    void clearEditor();

signals:
    void addedToSelection(photon::Clip*);
    void removedFromSelection(photon::Clip*);

protected:
    void showEvent(QShowEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SEQUENCEWIDGET_H
