#ifndef PHOTON_PIXELLAYOUTCOLLECTION_H
#define PHOTON_PIXELLAYOUTCOLLECTION_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT PixelLayoutCollection : public QObject
{
    Q_OBJECT
public:
    PixelLayoutCollection(QObject *parent = nullptr);
    ~PixelLayoutCollection();

    int layoutCount() const;
    PixelLayout *layoutAtIndex(uint) const;
    const QVector<PixelLayout*> &layouts() const;

signals:
    void layoutWillBeAdded(photon::PixelLayout *, int);
    void layoutWasAdded(photon::PixelLayout *, int);
    void layoutWillBeRemoved(photon::PixelLayout *, int);
    void layoutWasRemoved(photon::PixelLayout *, int);

public slots:
    void addLayout(photon::PixelLayout *);
    void removeLayout(photon::PixelLayout *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELLAYOUTCOLLECTION_H
