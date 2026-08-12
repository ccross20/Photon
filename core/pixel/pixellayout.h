#ifndef PHOTON_PIXELLAYOUT_H
#define PHOTON_PIXELLAYOUT_H
#include "photon-global.h"
#include "pixel/canvas.h"
#include "project/projectresource.h"

namespace photon {


class PHOTONCORE_EXPORT PixelLayout : public QObject, public ProjectResource
{
    Q_OBJECT
public:

    PixelLayout();
    virtual ~PixelLayout();

    QByteArray uniqueId() const;


    QString name() const;
    void setName(const QString &);

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return "pixel-layout";}
    QString resourceName() const override{return name();}
    void setResourceName(const QString &t_name) override{setName(t_name);}
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override;

    void addSource(PixelSourceLayout *);
    void removeSource(PixelSourceLayout *);

    const QVector<PixelSourceLayout*> &sourceLayouts() const;

    QVector<PixelSource*> sources() const;

    virtual void process(ProcessContext &, double blend=1.0) const;

    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

signals:
    void sourceWillBeAdded(photon::PixelSourceLayout *, int);
    void sourceWasAdded(photon::PixelSourceLayout *, int);
    void sourceWillBeRemoved(photon::PixelSourceLayout *, int);
    void sourceWasRemoved(photon::PixelSourceLayout *, int);
    void layoutModified();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELLAYOUT_H
