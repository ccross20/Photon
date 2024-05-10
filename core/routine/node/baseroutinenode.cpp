#include "baseroutinenode.h"

namespace photon {

BaseRoutineNode::BaseRoutineNode(const QByteArray &id):keira::Node(id) {}

BaseRoutineNode::~BaseRoutineNode()
{

}

void BaseRoutineNode::initializeContext(QOpenGLContext *, Canvas *)
{

}

void BaseRoutineNode::canvasResized(QOpenGLContext *, Canvas *)
{

}

} // namespace photon
