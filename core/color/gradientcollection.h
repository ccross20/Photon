#ifndef PHOTON_GRADIENTCOLLECTION_H
#define PHOTON_GRADIENTCOLLECTION_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT GradientCollection : public QObject
{
    Q_OBJECT
public:
    GradientCollection(QObject *parent = nullptr);
    ~GradientCollection();

    int gradientCount() const;
    GradientResource *gradientAtIndex(uint) const;
    GradientResource *findGradientWithId(const QByteArray &) const;
    GradientResource *findGradientWithName(const QString &) const;
    const QVector<GradientResource*> &gradients() const;

signals:
    void gradientWillBeAdded(photon::GradientResource *, int);
    void gradientWasAdded(photon::GradientResource *, int);
    void gradientWillBeRemoved(photon::GradientResource *, int);
    void gradientWasRemoved(photon::GradientResource *, int);

public slots:
    void addGradient(photon::GradientResource *);
    void removeGradient(photon::GradientResource *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_GRADIENTCOLLECTION_H
