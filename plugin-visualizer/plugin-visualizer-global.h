#ifndef PLUGINVISUALIZER_GLOBAL_H
#define PLUGINVISUALIZER_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QVector3D>
#include <QQuaternion>
#include <QMatrix4x4>
#include <QOpenGLContext>

#if defined(PLUGINVISUALIZER_LIBRARY)
#  define PLUGINVISUALIZER_EXPORT Q_DECL_EXPORT
#else
#  define PLUGINVISUALIZER_EXPORT Q_DECL_IMPORT
#endif

namespace photon {


enum DirtyModes
{
    Clean = 0,
    Dirty_Init = 0x1,
    Dirty_Destroy = 0x2,
    Dirty_Rebuild = 0x4,
    Dirty_Draw = 0x8
};

class AbstractComponent;
class Entity;
class Camera;

class DrawContext
{
public:
    DrawContext(QOpenGLContext *context, Camera *camera):camera(camera),openGLContext(context){}

    Camera *camera;
    QOpenGLContext *openGLContext;

};

}


#endif // PLUGINVISUALIZER_GLOBAL_H
