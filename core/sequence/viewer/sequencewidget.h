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

public slots:
    void togglePlay(bool);
    void rewind();
    void setScale(double);
    void gotoTime(double);

private slots:
    void tick();
    void detailsSplitterMoved(int, int);
    void editorSplitterMoved(int, int);
    void horizontalSplitterMoved(int, int);
    void selectionChanged();
    void xOffsetChanged(int);
    void selectEffect(photon::ChannelEffect *);
    void selectFalloff(photon::FalloffEffect *);
    void clearEditor();

signals:
    void addedToSelection(photon::Clip*);
    void removedFromSelection(photon::Clip*);

protected:
    void showEvent(QShowEvent*);

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SEQUENCEWIDGET_H
