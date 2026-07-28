#ifndef PHOTON_CANVASOUTPUTNODE_H
#define PHOTON_CANVASOUTPUTNODE_H

#include <QVector>
#include <QColor>
#include <QPointF>
#include <QMutex>
#include "model/node.h"
#include "photon-global.h"
#include "graph/parameter/rhitextureparameter.h"

namespace keira { class ButtonParameter; }

namespace photon {

class PixelLayoutListParameter;
class CanvasDmxSampler;
class ProcessContext;

// A canvas output/sink point. Whatever texture is wired in is this output's result
// (the auto-created "Output" feeds the preview). Each output can also be assigned
// pixel layouts — it samples ITS input texture at those layouts' pixel positions
// and drives their DMX, so one graph can drive different fixtures at different
// points in the flow. See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasOutputNode : public keira::Node
{
public:
    const static QByteArray TextureInput;
    const static QByteArray Layouts;
    const static QByteArray ViewPreview;

    CanvasOutputNode();
    ~CanvasOutputNode();

    void createParameters() override;
    void buttonClicked(const keira::Parameter *) override;
    // Main thread (inside the canvas frame): stashes the input texture and, if
    // layouts are assigned, records a GPU gather of the input at their positions.
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

    // The texture connected to this node's input on the last evaluation.
    RhiTextureData inputTexture() const { return m_lastTexture; }

    // Main thread, after the frame ends: pulls the gathered colours from the GPU.
    void collectGatheredColors() const;
    // Worker thread: writes the gathered colours to DMX via the assigned layouts.
    void writeDmx(ProcessContext &context) const;

private:
    QVector<class PixelLayout *> selectedLayouts() const;   // resolves ids -> layouts (main or worker)
    QVector<QPointF> buildSampleUVs() const;                // sample points of the assigned layouts

    RhiTextureParameter *m_textureInput = nullptr;
    PixelLayoutListParameter *m_layoutsParam = nullptr;
    keira::ButtonParameter *m_viewButton = nullptr;

    mutable RhiTextureData m_lastTexture;

    // GPU gather state (main-thread) + the colours it produced (shared to worker).
    mutable CanvasDmxSampler *m_sampler = nullptr;
    mutable QVector<QColor> m_gatheredColors;
    mutable QMutex m_gatheredMutex;
    mutable bool m_gatheredThisFrame = false;
};

} // namespace photon

#endif // PHOTON_CANVASOUTPUTNODE_H
