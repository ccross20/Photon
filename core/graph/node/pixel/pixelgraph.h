#ifndef PIXELGRAPH_H
#define PIXELGRAPH_H
#include "model/subgraphnode.h"
#include "photon-global.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/integerparameter.h"
#include "data/dmxtimemachine.h"


namespace photon {

class GraphContextNode;

class PHOTONCORE_EXPORT PixelGraph: public keira::SubGraphNode
{
public:
    const static QByteArray Pixels;
    const static QByteArray Enabled;
    const static QByteArray PixelSubGraphId;

    PixelGraph();
    ~PixelGraph();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    void prepForEvaluation() override;

    static keira::NodeInformation info();


    virtual void readFromJson(const QJsonObject &, keira::NodeLibrary *library) override;

protected:
    keira::NodeLibrary *nodeLibrary() const override;
    void parameterWasModified(keira::Parameter*) override;

private:
    keira::IntegerParameter *m_priortyParam;
    keira::BooleanParameter *m_enabledParam;
    keira::BooleanParameter *m_useTimeMachineParam;
    PixelListParameter *m_pixelsParam;
    GraphContextNode *m_globalsNode;
    // The default "Set Pixel Color" node seeded in the constructor - tracked
    // so readFromJson can remove it the same way it does m_globalsNode,
    // rather than leaving a stray extra node once a saved graph (which
    // already has its own nodes) is loaded in over it. Null if the node
    // library couldn't create it (e.g. plugin-nodes not loaded).
    keira::Node *m_seedSetColorNode = nullptr;
    DMXTimeMachine *m_timeMachine;
};

} // namespace photon

#endif // PIXELGRAPH_H
