#ifndef SURFACENODE_H
#define SURFACENODE_H

#include "model/subgraphnode.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SurfaceNode : public keira::SubGraphNode
{
public:
    const static QByteArray InputDMX;
    const static QByteArray OutputDMX;

    SurfaceNode();
    ~SurfaceNode();

    Surface *surface() const;
    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    void buttonClicked(const keira::Parameter *) override;

    static keira::NodeInformation info();


    void readFromJson(const QJsonObject &, keira::NodeLibrary *library) override;
    void writeToJson(QJsonObject &) const override;



private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACENODE_H
