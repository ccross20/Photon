#ifndef PHOTON_TIMELINEHEADER_H
#define PHOTON_TIMELINEHEADER_H

#include <QWidget>
#include "photon-global.h"

class QLabel;

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

protected:
     virtual void paintEvent(QPaintEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

class PHOTONCORE_EXPORT LayerGroupHeader : public LayerHeader
{
    Q_OBJECT
public:
    LayerGroupHeader(LayerGroup *);
    ~LayerGroupHeader();
    LayerGroup *group() const;

    void buildLayout() override;

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

public slots:
    void offsetChanged(int);

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
