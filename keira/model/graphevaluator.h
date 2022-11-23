#ifndef KEIRA_GRAPHEVALUATOR_H
#define KEIRA_GRAPHEVALUATOR_H

#include <QObject>
#include "keira-global.h"

namespace keira {

class KEIRA_EXPORT GraphEvaluator : public QObject
{
    Q_OBJECT
public:
    explicit GraphEvaluator(Graph *t_graph, QObject *parent = nullptr);
    ~GraphEvaluator();

    bool isEnabled() const;
public slots:
    void setGraph(keira::Graph*);
    void setIsEnabled(bool);

private slots:
    void graphUpdated();
    void timeout();

signals:
    void evaluationComplete();

private:
    class Impl;
    Impl *m_impl;

};

} // namespace keira

#endif // KEIRA_GRAPHEVALUATOR_H
