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

    // The node references a Surface owned by the Project rather than owning one:
    // several SurfaceNodes can drive the same surface, and the surface outlives
    // any one node. Resolves to nullptr if the id is unset or no longer in the
    // project's collection.
    Surface *surface() const;
    QByteArray surfaceId() const;
    void setSurfaceId(const QByteArray &);

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
