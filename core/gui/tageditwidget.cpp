#include <QMouseEvent>
#include <QPaintEvent>
#include "tageditwidget_p.h"
#include "tagpicker.h"

namespace photon {

TagWidget::TagWidget(const QString &t_tag): QWidget(),m_tag(t_tag)
{

}

QString TagWidget::tag() const
{
    return m_tag;
}

QSize TagWidget::sizeHint() const
{
    return QSize{100,20};
}

void TagWidget::paintEvent(QPaintEvent *t_event)
{
    QWidget::paintEvent(t_event);
}

void TagWidget::mousePressEvent(QMouseEvent *t_event)
{

}






TagEditWidget::TagEditWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->hLayout = new QHBoxLayout;
    setLayout(m_impl->hLayout);
}

TagEditWidget::~TagEditWidget()
{
    delete m_impl;
}

void TagEditWidget::setIsReadOnly(bool t_value)
{
    m_impl->isReadOnly = t_value;
}

bool TagEditWidget::isReadOnly() const
{
    return m_impl->isReadOnly;
}

QSize TagEditWidget::sizeHint() const
{
    return QSize{100,20};
}

void TagEditWidget::addTag(const QString &t_tag)
{
    auto tagWidget = new TagWidget{t_tag};

    m_impl->hLayout->addWidget(tagWidget);
    m_impl->tagWidgets.append(tagWidget);
}

void TagEditWidget::removeTag(const QString &)
{

}

void TagEditWidget::setTags(const QStringList &)
{

}

void TagEditWidget::mousePressEvent(QMouseEvent *event)
{
    TagPicker picker;

    picker.exec();
}

} // namespace photon
