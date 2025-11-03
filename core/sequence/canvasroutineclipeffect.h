#ifndef PHOTON_CANVASROUTINECLIPEFFECT_H
#define PHOTON_CANVASROUTINECLIPEFFECT_H

#include <QWidget>
#include "canvaseffect.h"
#include "view/viewer.h"
#include "view/scene.h"

namespace photon {

class PHOTONCORE_EXPORT RoutineEditor : public QWidget
{
public:
    RoutineEditor(Routine *);
    ~RoutineEditor();

private:
    keira::Scene *scene;
    keira::Viewer *viewer;
};



class PHOTONCORE_EXPORT CanvasRoutineClipEffect : public CanvasEffect
{
    Q_OBJECT
public:
    CanvasRoutineClipEffect();
    virtual ~CanvasRoutineClipEffect();

    void initializeContext(QOpenGLContext *, Canvas *) override;
    void canvasResized(QOpenGLContext *, Canvas *) override;
    Routine *routine() const;
    void evaluate(CanvasEffectEvaluationContext &) override;
    QWidget *createEditor() override;
    static ClipEffectInformation info();

    virtual void restore(Project &) override;
    virtual void readFromJson(const QJsonObject &, const LoadContext &) override;
    virtual void writeToJson(QJsonObject &) const override;

public slots:
    void nodeWasAdded(keira::Node *);
    void nodeWasRemoved(keira::Node *);
    void routineChannelUpdatedSlot(int index);
    void channelAddedSlot(int index);
    void channelRemovedSlot(int index);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASROUTINECLIPEFFECT_H
