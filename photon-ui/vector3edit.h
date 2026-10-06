#ifndef PHOTON_VECTOR3EDIT_H
#define PHOTON_VECTOR3EDIT_H

#include <QWidget>
#include "photon-ui-global.h"

namespace photon {

class PHOTONUI_EXPORT Vector3Edit : public QWidget
{
    Q_OBJECT
public:
    // What the three numbers are, which sets their unit and range: scene
    // positions and sizes are in metres, rotations in degrees.
    enum Kind { Distance, Angle };

    explicit Vector3Edit(Kind kind, QWidget *parent = nullptr);
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
