#ifndef SUBGRAPHNODE_H
#define SUBGRAPHNODE_H
#include <QHash>
#include "node.h"

namespace keira {

class KEIRA_EXPORT SubGraphNode : public Node
{
public:
    SubGraphNode(const QByteArray &id);
    virtual ~SubGraphNode();

    // NOTE: graph() here is the INNER graph this node wraps, hiding Node::graph()
    // (the outer graph this node lives in). Use Node::graph() for the outer one.
    Graph *graph() const{return m_graph;}
    virtual bool isContainer() const override {return true;}
    virtual Node *findNode(const QByteArray &query) const override;

    void drainCommandQueue() override;
    virtual void evaluate(EvaluationContext *context) const override;
    virtual void markDirty(int) override;
    void markClean() override;

    virtual void readFromJson(const QJsonObject &, NodeLibrary *library) override;
    virtual void writeToJson(QJsonObject &) const override;

    // Create a GraphInputNode of the type matching t_paramTypeId inside the inner
    // graph and return the outer mirror parameter it produces (or null if the type
    // isn't supported / no library). UI-initiated; runs synchronously so the caller
    // can immediately connect a wire to the returned port.
    Parameter *exposeInputForType(const QByteArray &paramTypeId);

protected:
    // The node library used to clone typed mirror parameters. keira has no global
    // library, so photon subgraphs override this to supply the app's library.
    // Returning null disables mirroring (e.g. headless).
    virtual NodeLibrary *nodeLibrary() const;

    // Reconcile our outer parameters against the inner graph's interface: add a
    // mirror parameter for each input/output port, remove mirrors for ports that
    // are gone. Safe to call from the UI thread — parameter mutations are queued
    // on the outer graph. Wired to the inner graph's interfaceChanged() signal.
    void reconcilePorts();

    // Relay outer input-mirror values into the input nodes of the given graph
    // (which may be the inner graph or a pool clone of it), matched by portId.
    void applyInputs(Graph *target) const;

    // Copy the output nodes of the given graph back into our outer output mirrors.
    void readOutputs(Graph *source) const;

private:
    // Detach a mirror parameter (dropping its outer connections) and retire it.
    void retireMirror(Parameter *);

    Graph *m_graph;
    QHash<QByteArray, Parameter*> m_inputMirrors;   // portId -> outer input param
    QHash<QByteArray, Parameter*> m_outputMirrors;  // portId -> outer output param
    // Removed mirrors are retired here rather than deleted immediately, so the
    // eval thread never dereferences a freed parameter.
    QVector<Parameter*> m_retiredMirrors;
    QMetaObject::Connection m_interfaceConn;
};

} // namespace keira

#endif // SUBGRAPHNODE_H
