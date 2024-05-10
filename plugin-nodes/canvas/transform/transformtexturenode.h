#ifndef PHOTON_TRANSFORMTEXTURENODE_H
#define PHOTON_TRANSFORMTEXTURENODE_H
#include "routine/node/baseroutinenode.h"
#include "graph/parameter/textureparameter.h"
#include "graph/parameter/point2dparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class TransformTextureNode : public BaseRoutineNode
{
public:
    TransformTextureNode();
    ~TransformTextureNode();


    void createParameters() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    TextureParameter *m_textureInputParam;
    TextureParameter *m_textureOutputParam;
    Point2DParameter *m_positionParam;
    Point2DParameter *m_scaleParam;
    Point2DParameter *m_centerParam;
    keira::DecimalParameter *m_rotationParam;
    OpenGLTexture *m_texture;



};

} // namespace photon

#endif // PHOTON_TRANSFORMTEXTURENODE_H
