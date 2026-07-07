#ifndef SURFACEGIZMO_H
#define SURFACEGIZMO_H

#include <QObject>
#include "photon-global.h"
#include "gizmoproperty.h"

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

    // Describes a value this gizmo publishes onto the surface value bus, read by
    // GizmoValueNode in the graph. Decouples the graph from concrete gizmo types.
    struct GizmoOutput
    {
        QByteArray id;
        QString name;
        GizmoProperty::Type type = GizmoProperty::Number;
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

    // Data-driven property model. Concrete gizmos register their persisted,
    // editable configuration here; the base handles serialization and the
    // inspector/QML layers read the definitions generically.
    GizmoProperty *addProperty(const QByteArray &id, const QString &name, GizmoProperty::Type type,
                               const QVariant &defaultValue, const QVariantHash &metadata = {});
    GizmoProperty *property(const QByteArray &id) const;
    const QVector<GizmoProperty*> &properties() const;
    QVariant propertyValue(const QByteArray &id) const;
    void setPropertyValue(const QByteArray &id, const QVariant &value);

    // Value-bus outputs. Concrete gizmos override to expose their live values
    // (a slider's position, a toggle's state, ...) to the graph.
    virtual QVector<GizmoOutput> outputs() const;
    virtual QVariant outputValue(const QByteArray &portId) const;

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
    void propertyChanged(const QByteArray &id);

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // SURFACEGIZMO_H
