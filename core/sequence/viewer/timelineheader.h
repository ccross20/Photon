#ifndef PHOTON_TIMELINEHEADER_H
#define PHOTON_TIMELINEHEADER_H

#include <QWidget>
#include "photon-global.h"

class QLabel;
class QToolButton;
class QContextMenuEvent;
class QMouseEvent;

namespace photon {

class PHOTONCORE_EXPORT LayerHeader : public QWidget
{
    Q_OBJECT
public:
    LayerHeader(Layer *);
    virtual ~LayerHeader();
    QLabel *label() const;
    Layer *layer() const;
    virtual void buildLayout();

    virtual QSize sizeHint() const override;

    void setActive(bool);

signals:
    void activated(photon::Layer *);

private slots:
    void renameClicked();
    void muteToggled(bool);

protected:
     virtual void paintEvent(QPaintEvent *event) override;
     virtual void contextMenuEvent(QContextMenuEvent *event) override;
     virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
     virtual void mousePressEvent(QMouseEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT TimelineHeader : public QWidget
{
    Q_OBJECT
public:
    explicit TimelineHeader(QWidget *parent = nullptr);
    ~TimelineHeader();

    void setSequence(Sequence *);
    Sequence *sequence() const;

signals:
    void editLayer(photon::Layer *);
    // A layer header was clicked - it should become the active layer.
    void layerActivated(photon::Layer *);

public slots:
    void offsetChanged(int);
    // Highlights the active layer's header (null clears it).
    void setActiveLayer(photon::Layer *);

private slots:
    void layerUpdated(photon::Layer *);
    void layerAdded(photon::Layer *);
    void layerRemoved(photon::Layer *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINEHEADER_H
