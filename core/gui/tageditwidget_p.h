#ifndef TAGEDITWIDGET_P_H
#define TAGEDITWIDGET_P_H

#include <QHBoxLayout>
#include "tageditwidget.h"

namespace photon {

class TagWidget : public QWidget
{
public:
    TagWidget(const QString &);

    QString tag() const;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;


private:
    QString m_tag;
};




class TagEditWidget::Impl
{
public:

    QVector<TagWidget*> tagWidgets;
    QHBoxLayout *hLayout;
    bool isReadOnly = false;
};


}



#endif // TAGEDITWIDGET_P_H
