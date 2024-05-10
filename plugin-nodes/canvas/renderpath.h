#ifndef PHOTON_RENDERPATH_H
#define PHOTON_RENDERPATH_H

#include "routine/node/baseroutinenode.h"
#include "graph/parameter/textureparameter.h"
#include "graph/parameter/pathparameter.h"
#include "graph/parameter/colorparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"

namespace photon {

class RenderPath : public BaseRoutineNode
{
public:
    RenderPath();
    ~RenderPath();

    void createParameters() override;
    void initializeContext(QOpenGLContext *, Canvas *) override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    TextureParameter *m_textureInputParam;
    TextureParameter *m_textureOutputParam;
    PathParameter *m_pathInputParam;
    keira::OptionParameter *m_sizeParam;
    keira::DecimalParameter *m_alphaParam;
    ColorParameter *m_colorParam;
    OpenGLTexture *m_texture;
};

} // namespace photon

#endif // PHOTON_RENDERPATH_H
