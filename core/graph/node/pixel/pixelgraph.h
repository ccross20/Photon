#ifndef PIXELGRAPH_H
#define PIXELGRAPH_H
#include "model/subgraphnode.h"
#include "photon-global.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/integerparameter.h"
#include "data/dmxtimemachine.h"


namespace photon {

class PixelGlobalsNode;

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
    void parameterWasAdded(keira::Parameter*) override;
    void parameterWasRemoved(keira::Parameter*) override;
    void parameterWasModified(keira::Parameter*) override;

private:
    keira::IntegerParameter *m_priortyParam;
    keira::BooleanParameter *m_enabledParam;
    keira::BooleanParameter *m_useTimeMachineParam;
    PixelListParameter *m_pixelsParam;
    PixelGlobalsNode *m_globalsNode;
    QVector<keira::Parameter*> m_passThroughParams;
    QVector<keira::Parameter*> m_globalsParams;
    DMXTimeMachine *m_timeMachine;
};

} // namespace photon

#endif // PIXELGRAPH_H
