#ifndef PHOTON_CHANNELEFFECTEDITOR_H
#define PHOTON_CHANNELEFFECTEDITOR_H

#include "photon-global.h"
#include <QWidget>
#include <QToolBar>

class QGraphicsItem;

namespace photon {
class ChannelEffect;

class PHOTONCORE_EXPORT ChannelEffectEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelEffectEditor(ChannelEffect *, QWidget *parent = nullptr);
    virtual ~ChannelEffectEditor();

    ChannelEffect *effect() const;
    QToolBar *toolbar() const;
    QPointF scale() const;
    QPointF offset() const;

    QTransform transform() const;

    void addItem(QGraphicsItem *);
    void addWidget(QWidget *, const QString &name);

signals:
    void scaleChanged(double);
    void offsetChanged(double);

public slots:
    void setScale(double);
    void setOffset(double);
    void clipStartUpdated(double);
    void clipDurationUpdated(double);
    void effectUpdated(photon::ChannelEffect *);

private slots:
    void relayoutSlot();

protected:
    virtual void relayout(const QRectF &);
    void resizeEvent(QResizeEvent *) override;
    void paintEvent(QPaintEvent *) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_CHANNELEFFECTEDITOR_H
