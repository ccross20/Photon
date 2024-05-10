#ifndef PHOTON_MASKNODE_H
#define PHOTON_MASKNODE_H

#include "routine/node/baseroutinenode.h"
#include "graph/parameter/textureparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

class MaskNode : public BaseRoutineNode
{
public:
    MaskNode();

    void createParameters() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    TextureParameter *m_textureAParam;
    TextureParameter *m_textureBParam;
    TextureParameter *m_textureOutputParam;
    keira::OptionParameter *m_modeParam;
    OpenGLTexture *m_texture;
    OpenGLShader *m_maskShader = nullptr;
    OpenGLShader *m_invertShader = nullptr;
    OpenGLShader *m_outputShader = nullptr;
    OpenGLPlane *m_plane = nullptr;
};

} // namespace photon

#endif // PHOTON_MASKNODE_H
