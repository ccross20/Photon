#ifndef PHOTON_BASEROUTINENODE_H
#define PHOTON_BASEROUTINENODE_H

#include "photon-global.h"
#include "model/node.h"
#include "opengl/openglframebuffer.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

class PHOTONCORE_EXPORT BaseRoutineNode : public keira::Node
{
public:
    BaseRoutineNode(const QByteArray &id);
    virtual ~BaseRoutineNode();

    virtual void initializeContext(QOpenGLContext *, Canvas *);
    virtual void canvasResized(QOpenGLContext *, Canvas *);
};

} // namespace photon

#endif // PHOTON_BASEROUTINENODE_H
