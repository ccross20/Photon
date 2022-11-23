#ifndef PHOTON_VECTOR3EDIT_H
#define PHOTON_VECTOR3EDIT_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Vector3Edit : public QWidget
{
    Q_OBJECT
public:
    explicit Vector3Edit(QWidget *parent = nullptr);
    ~Vector3Edit();

    void setValue(const QVector3D &);
    QVector3D value() const;

signals:
    void valueChanged(QVector3D );

private slots:
    void inputChanged(double);

protected:
    virtual void changeEvent(QEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_VECTOR3EDIT_H
