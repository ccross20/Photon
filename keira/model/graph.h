#ifndef KEIRA_GRAPH_H
#define KEIRA_GRAPH_H

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
    Parameter *findParameter(const QByteArray &);
    const QVector<Node*> &nodes() const;
    const QVector<Node*> LoopNodes() const;

    virtual void evaluate(EvaluationContext *) const;
    virtual void evaluateAll(EvaluationContext *) const;
    void connectParameters(const QByteArray &, const QByteArray &);
    void disconnectParameters(const QByteArray &, const QByteArray &);
    void connectParameters(Parameter *, Parameter *);
    void disconnectParameters(Parameter *, Parameter *);
    void disconnectNode(Node *);
    void updateNodePosition(Node*);

    void markDirty();
    void markClean();
    bool isDirty() const;

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

private:
    friend class Node;

    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_GRAPH_H
