#ifndef SIMPLESWATCH_H
#define SIMPLESWATCH_H

#include "photon-global.h"
#include <QWidget>

namespace photon {

class PHOTONCORE_EXPORT SimpleSwatch : public QWidget
{
    Q_OBJECT
public:
    explicit SimpleSwatch(const QColor &color = QColor(Qt::black), QWidget *parent = nullptr);
    ~SimpleSwatch() override;
    const QColor &color() const;

public slots:
    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
    Q_DISABLE_COPY(SimpleSwatch)

};

} // namespace exo

#endif // SIMPLESWATCH_H
