#include <QPainter>
#include <QPaintEvent>
#include "timebar.h"

namespace photon {

TimeBar::TimeBar(QWidget *parent)
    : QWidget{parent}
{
    setMinimumHeight(30);
    setMaximumHeight(30);
}

void TimeBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setClipRect(event->rect());
    painter.drawPixmap(0,0,m_cachedMarkings);
}

void TimeBar::setScale(double t_value)
{
    m_scale = t_value;
    redrawCache();
    update();
}

void TimeBar::setOffset(double t_value)
{
    m_offset = -t_value;
    redrawCache();
    update();
}

void TimeBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    redrawCache();
    update();
}

void TimeBar::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);

    emit changeTime((event->pos().x() - m_offset) / m_scale);
}

void TimeBar::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);

    emit changeTime((event->pos().x() - m_offset) / m_scale);
}

void TimeBar::redrawCache()
{
    m_cachedMarkings = QPixmap(width(), height());
    QPainter painter(&m_cachedMarkings);

    painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
    QPen pen(Qt::white,0); // zero width pen is cosmetic pen
    painter.setPen(pen);

    QRectF rulerRect = this->rect();

    painter.fillRect(rulerRect,Qt::gray);

    double rulerUnit = 20;

    double markSpacing = rulerUnit * m_scale;

    double inverseSpacing = (1.0/markSpacing)*rulerUnit;
    double zoomMultiple = 1.0;

    if(m_scale >= 10.0)
        zoomMultiple *= .1;
    else if(m_scale < 1.0)
        zoomMultiple *= 10;

    if(m_scale < .1)
        zoomMultiple *= 100;

    markSpacing *= zoomMultiple;
    double unitDivisor = 1.0 / zoomMultiple;


    //double markStartPos = (static_cast<int>(round(m_origin)) % static_cast<int>(round(markSpacing))) - markSpacing;

    double markStartPos = std::remainder(m_offset, markSpacing) - markSpacing;

    //qDebug() << markStartPos << "  " << markSpacing;
    double maxSize = rulerRect.width() ;

    double markSubSpacing = markSpacing * .1;
    double markPos = markStartPos;
    double submarkDepth = 15.0;

    double rulerDepth = height();
    //qDebug() << "Zoom: " << m_rulerZoom << " Spacing: " << markSpacing;



    while(markPos < maxSize && markSubSpacing > 4)
    {
        painter.drawLine(static_cast<int>(markPos), static_cast<int>(submarkDepth), static_cast<int>(markPos), rulerDepth);
        markPos += markSubSpacing;
    }
    markPos = markStartPos;
    while(markPos < maxSize)
    {
        painter.drawLine(static_cast<int>(markPos), 0, static_cast<int>(markPos), rulerDepth);
        double time = (round(((markPos - m_offset)*inverseSpacing) * unitDivisor) * zoomMultiple);

        double minutes = floor(time/60.0);
        double seconds = time - (minutes * 60);

        painter.drawText(static_cast<int>(markPos)+ 1,10, QString::number(minutes) + ":" + QString::number(seconds).leftJustified(2,'0'));
        //painter.drawText(static_cast<int>(markPos)+ 1,10, QString::number((((markPos - m_origin)*inverseSpacing) * unitDivisor) * zoomMultiple));
        markPos += markSpacing;
    }

}

} // namespace photon
