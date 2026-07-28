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

    // Optional finite range; enables the pad's absolute mode and the axis
    // fields' fill bars. Unset (default) = relative scrubbing.
    void setRange(const QPointF &minimum, const QPointF &maximum);

signals:
    void valueChanged(QPointF );

protected:
    virtual void changeEvent(QEvent *event) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_POINTEDIT_H
