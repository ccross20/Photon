#ifndef SUBGRAPHNODE_H
#define SUBGRAPHNODE_H
#include "node.h"

namespace keira {

class KEIRA_EXPORT SubGraphNode : public Node
{
public:
    SubGraphNode(const QByteArray &id);
    virtual ~SubGraphNode();

    Graph *graph() const{return m_graph;}
    virtual bool isContainer() const override {return true;}
    virtual Node *findNode(const QByteArray &query) const override;

    void drainCommandQueue() override;
    virtual void evaluate(EvaluationContext *context) const override;
    virtual void markDirty(int) override;
    void markClean() override;

    virtual void readFromJson(const QJsonObject &, NodeLibrary *library) override;
    virtual void writeToJson(QJsonObject &) const override;

private:
    Graph *m_graph;
};

} // namespace keira

#endif // SUBGRAPHNODE_H
