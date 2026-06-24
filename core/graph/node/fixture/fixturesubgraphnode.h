#ifndef FIXTURESUBGRAPHNODE_H
#define FIXTURESUBGRAPHNODE_H
#include <QObject>
#include "model/subgraphnode.h"
#include "photon-global.h"
#include "graph/parameter/fixturelistparameter.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class FixtureGlobalsNode;

class PHOTONCORE_EXPORT FixtureSubGraphNode: public keira::SubGraphNode
{
public:
    const static QByteArray Fixtures;
    const static QByteArray Enabled;
    const static QByteArray FixtureSubGraphId;

    FixtureSubGraphNode();
    ~FixtureSubGraphNode();

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
    void syncSubgraphPool(int count) const;

    keira::IntegerParameter *m_priortyParam;
    keira::BooleanParameter *m_enabledParam;
    FixtureListParameter *m_fixturesParam;
    FixtureGlobalsNode *m_globalsNode;
    QVector<keira::Parameter*> m_passThroughParams;
    QVector<keira::Parameter*> m_globalsParams;

    // One cloned subgraph per fixture — each has independent node parameter
    // instances so the parallel fixture loop has no shared write state.
    mutable QVector<keira::Graph*> m_subgraphPool;
    mutable QVector<FixtureGlobalsNode*> m_globalsPool;

    // Set when the source subgraph changes (e.g. a node's parameter is edited),
    // so the pool is re-cloned on the next evaluate(). Without this, node-internal
    // parameter edits never reach the per-fixture clones.
    mutable bool m_poolStale = true;
    QMetaObject::Connection m_dirtyConn;
    QMetaObject::Connection m_connectConn;
    QMetaObject::Connection m_disconnectConn;
};

} // namespace photon

#endif // FIXTURESUBGRAPHNODE_H
