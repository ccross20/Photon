#ifndef KEIRA_GRAPHEVALUATOR_H
#define KEIRA_GRAPHEVALUATOR_H

#include <QObject>
#include <functional>
#include "keira-global.h"
#include "model/frametime.h"

namespace keira {

struct EvaluationContext;

class KEIRA_EXPORT GraphEvaluator : public QObject
{
    Q_OBJECT
public:
    using ContextFactory   = std::function<EvaluationContext*(const FrameTime &)>;
    using PostEvalCallback = std::function<void()>;

    explicit GraphEvaluator(Graph *t_graph,
                            ContextFactory factory = {},
                            PostEvalCallback postEval = {},
                            QObject *parent = nullptr);
    ~GraphEvaluator();

    bool isEnabled() const;
    FrameTime currentTime() const;

    void setContextFactory(ContextFactory factory);
    void setPostEvalCallback(PostEvalCallback callback);

public slots:
    void setGraph(keira::Graph *graph);
    void setIsEnabled(bool enabled);
    void setBpm(double bpm);
    void setBeatsPerBar(int beatsPerBar);
    void tap();

signals:
    void evaluationComplete();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_GRAPHEVALUATOR_H
