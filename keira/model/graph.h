#ifndef KEIRA_GRAPH_H
#define KEIRA_GRAPH_H

#include <functional>
#include "keira-global.h"

namespace keira {

class KEIRA_EXPORT Graph : public QObject
{
    Q_OBJECT
public:
    explicit Graph(QObject *parent = nullptr);
    ~Graph();

    void addNode(Node *);
    void removeNode(Node *);
    QByteArray uniqueId() const;
    Node *findNode(const QByteArray &) const;
    QVector<Node *> nodeHierarchy() const;
    Parameter *findParameter(const QByteArray &);
    const QVector<Node*> &nodes() const;
    QByteArray graphTypeId() const;
    void setGraphTypeId(const QByteArray &);

    // Drains and applies all commands queued by the UI thread.
    // Called by the eval thread at the start of each frame.
    void drainCommandQueue();

    // Enqueue arbitrary work to run on the eval thread at the next frame start,
    // for structural changes that must not race the evaluator (e.g. adding or
    // removing a node's parameters while it is being evaluated).
    void runCommand(std::function<void()>);

    void prepForEvaluation();
    virtual void evaluate(EvaluationContext *) const;
    virtual void evaluateAll(EvaluationContext *) const;
    void connectParameters(const QByteArray &, const QByteArray &);
    void disconnectParameters(const QByteArray &, const QByteArray &);
    void connectParameters(Parameter *, Parameter *);
    void disconnectParameters(Parameter *, Parameter *);
    void disconnectNode(Node *);
    void updateNodePosition(Node*);

    void markDirty(int);
    void markClean();
    bool isDirty() const;

    Node *parentNode() const;
    void setParentNode(Node *);

    void setName(const QString &);
    QString name() const;
    QString familyName() const;

    virtual void readFromJson(const QJsonObject &, NodeLibrary *library);
    virtual void writeToJson(QJsonObject &) const;

protected:

    virtual void nodeAdded(keira::Node *);
    virtual void nodeRemoved(keira::Node *);

signals:

    void nodeWasAdded(keira::Node *);
    void nodeWasRemoved(keira::Node *);
    void nodePositionUpdated(keira::Node *);
    void parametersWereConnected(keira::Parameter *t_out, keira::Parameter *t_in);
    void parametersWereDisconnected(keira::Parameter *t_out, keira::Parameter *t_in);
    void dirtyStateChanged();
    // A node's parameter set changed at runtime (ports added/removed); the view
    // should rebuild that node's ports.
    void nodePortsChanged(keira::Node *);

private:
    friend class Node;

    // Non-locking implementations — called only from within drainCommandQueue()
    // or from other internal methods already on the eval thread.
    void addNodeInternal(Node *);
    void removeNodeInternal(Node *);
    void connectParametersInternal(Parameter *, Parameter *);
    void disconnectParametersInternal(Parameter *, Parameter *);
    void disconnectNodeInternal(Node *);
    void resortGraphInternal();

    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_GRAPH_H
