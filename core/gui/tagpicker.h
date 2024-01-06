#ifndef PHOTON_TAGPICKER_H
#define PHOTON_TAGPICKER_H

#include <QDialog>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT TagPicker : public QDialog
{
    Q_OBJECT
public:
    explicit TagPicker(QWidget *parent = nullptr);
    ~TagPicker();

    QStringList tags() const;

public slots:
    void accept() override;
    void reject() override;

signals:
    void tagsPicked(QStringList);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TAGPICKER_H
