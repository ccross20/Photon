#ifndef SURFACEGIZMO_H
#define SURFACEGIZMO_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SurfaceGizmo : public QObject
{
    Q_OBJECT
public:

    const static QByteArray GizmoPressId;
    const static QByteArray GizmoReleaseId;

   struct GizmoHistoryEvent
    {
        QByteArray id;
        QVariant value;
        double time;
    };



    explicit SurfaceGizmo(QByteArray type, QObject *parent = nullptr);
    virtual ~SurfaceGizmo();
    void setId(const QByteArray&);
    QByteArray id() const;
    QByteArray uniqueId() const;
    QByteArray type() const;

    void addHistoryEvent(const QByteArray &id, const QVariant &value = QVariant());
    GizmoHistoryEvent lastHistoryEvent() const;
    QVector<GizmoHistoryEvent> historyEvents() const;

    virtual void processChannels(ProcessContext &);

    virtual SurfaceGizmoWidget *createWidget(SurfaceMode mode);

    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

    void markChanged();

protected:
    void setType(const QByteArray t_type);

signals:
    void gizmoUpdated();

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // SURFACEGIZMO_H
