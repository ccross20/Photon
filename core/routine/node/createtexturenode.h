#ifndef PHOTON_CREATETEXTURENODE_H
#define PHOTON_CREATETEXTURENODE_H

#include "baseroutinenode.h"
#include "photon-global.h"
namespace photon {

class PHOTONCORE_EXPORT CreateTextureNode : public BaseRoutineNode
{
public:
    const static QByteArray SizeInput;
    const static QByteArray ColorInput;
    const static QByteArray TransparentInput;

    CreateTextureNode();
    ~CreateTextureNode();

    void initializeContext(QOpenGLContext *, Canvas *) override;
    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CREATETEXTURENODE_H
