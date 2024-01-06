#ifndef PHOTON_POINTEDIT_H
#define PHOTON_POINTEDIT_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT PointEdit : public QWidget
{
    Q_OBJECT
public:
    explicit PointEdit(QWidget *parent = nullptr);
    ~PointEdit();

    void setValue(const QPointF &);
    QPointF value() const;

signals:
    void valueChanged(QPointF );

private slots:
    void inputChanged(double);

protected:
    virtual void changeEvent(QEvent *event) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_POINTEDIT_H
