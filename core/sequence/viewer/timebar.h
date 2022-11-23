#ifndef PHOTON_TIMEBAR_H
#define PHOTON_TIMEBAR_H

#include <QWidget>

namespace photon {

class TimeBar : public QWidget
{
    Q_OBJECT
public:
    explicit TimeBar(QWidget *parent = nullptr);

public slots:
    void setScale(double);
    void setOffset(double);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

signals:
    void changeTime(double);

private:
    void redrawCache();

    QPixmap m_cachedMarkings;
    double m_scale = 5.0;
    double m_offset = 0.0;

};

} // namespace photon

#endif // PHOTON_TIMEBAR_H
