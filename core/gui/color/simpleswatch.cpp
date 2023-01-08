#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include "simpleswatch.h"

namespace photon {

class SimpleSwatch::Impl
{
public:
    Impl(const QColor &color);

    QColor color;
};

SimpleSwatch::Impl::Impl(const QColor &color) : color(color)
{

}

SimpleSwatch::SimpleSwatch(const QColor &color, QWidget *parent) : QWidget(parent), m_impl(new Impl(color))
{

}

SimpleSwatch::~SimpleSwatch()
{
    delete m_impl;
}

const QColor &SimpleSwatch::color() const
{
    return m_impl->color;
}

void SimpleSwatch::setColor(const QColor &color)
{
    if(m_impl->color == color)
        return;
    m_impl->color = color;
    update();
}

void SimpleSwatch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
/*
    if(m_impl->color.alpha() < 1.0)
        p.fillRect(rect(), exoApp->settings()->alphaBackgroundBrush());
        */
    p.fillRect(rect(), QBrush(m_impl->color));

}

} // namespace exo

