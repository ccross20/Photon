#ifndef GIZMOFACTORY_H
#define GIZMOFACTORY_H
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT GizmoFactory
{
public:

    struct GizmoInfo
    {
        GizmoInfo(const QString &t_name, const QString &t_id):name(t_name),id(t_id){}
        QString name;
        QString id;
    };


    GizmoFactory();

    static QVector<GizmoInfo> getGizmoInfo();
    static SurfaceGizmo *createGizmo(const QByteArray &);


};

} // namespace photon

#endif // GIZMOFACTORY_H
