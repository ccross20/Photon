#ifndef PHOTON_SEQUENCECURVEEDITOR_H
#define PHOTON_SEQUENCECURVEEDITOR_H

#include <QVBoxLayout>
#include <QWidget>
#include <QPainterPath>
#include "photon-global.h"

namespace photon {

class CurveViewport : public QWidget
{
    Q_OBJECT
public:
    CurveViewport();

public slots:
    void setViewBounds(const QRectF &rect);
    void clear();
    void addEffect(photon::ChannelEffect *);
    void timeChanged(double);
    void durationChanged(double);
    void channelUpdated(photon::Channel*);
    void setXOffset(int);
    void setScale(double);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    void rebuildPaths();
    void remakeSceneBounds();

    ChannelEffect *m_effect = nullptr;
    Clip *m_clip = nullptr;
    QPainterPath m_path;
    QPainterPath m_channelPath;
    QRectF m_viewBounds;
    QRectF m_sceneBounds;
    double m_scale = 5.0;
    double m_offset = 0.0;
    bool m_pathsDirty = true;
};

class ClipEffectEditor : public QWidget
{
    Q_OBJECT
public:
    explicit ClipEffectEditor(QWidget *parent = nullptr);

signals:

public slots:
    void selectEffect(photon::ChannelEffect *);
    void clearSelection();
    void setXOffset(int);
    void setScale(double);

private:
    QWidget *m_effectEditor = nullptr;
    CurveViewport *m_viewport;
    QVBoxLayout *m_layout;

};

} // namespace photon

#endif // PHOTON_SEQUENCECURVEEDITOR_H
