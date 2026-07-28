#include "canvaslevelsnode.h"

namespace photon {

const QByteArray CanvasLevelsNode::Input = "input";
const QByteArray CanvasLevelsNode::InputBlack = "inputBlack";
const QByteArray CanvasLevelsNode::InputWhite = "inputWhite";
const QByteArray CanvasLevelsNode::Gamma = "gamma";
const QByteArray CanvasLevelsNode::OutputBlack = "outputBlack";
const QByteArray CanvasLevelsNode::OutputWhite = "outputWhite";
const QByteArray CanvasLevelsNode::Output = "output";

keira::NodeInformation CanvasLevelsNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasLevelsNode;});
    toReturn.name = "Levels";
    toReturn.nodeId = "photon.canvas.levels";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasLevelsNode::CanvasLevelsNode() : BaseCanvasNode("photon.canvas.levels")
{
    setName("Levels");
}

void CanvasLevelsNode::createParameters()
{
    m_input = new RhiTextureParameter(Input, "Canvas", RhiTextureData{}, keira::AllowSingleInput);
    addParameter(m_input);

    m_inputBlack = new keira::DecimalParameter(InputBlack, "Input Black", 0.0);
    m_inputBlack->setMinimum(0.0);
    m_inputBlack->setMaximum(1.0);
    addParameter(m_inputBlack);

    m_inputWhite = new keira::DecimalParameter(InputWhite, "Input White", 1.0);
    m_inputWhite->setMinimum(0.0);
    m_inputWhite->setMaximum(1.0);
    addParameter(m_inputWhite);

    m_gamma = new keira::DecimalParameter(Gamma, "Gamma", 1.0);
    m_gamma->setMinimum(0.01);
    m_gamma->setMaximum(10.0);
    addParameter(m_gamma);

    m_outputBlack = new keira::DecimalParameter(OutputBlack, "Output Black", 0.0);
    m_outputBlack->setMinimum(0.0);
    m_outputBlack->setMaximum(1.0);
    addParameter(m_outputBlack);

    m_outputWhite = new keira::DecimalParameter(OutputWhite, "Output White", 1.0);
    m_outputWhite->setMinimum(0.0);
    m_outputWhite->setMaximum(1.0);
    addParameter(m_outputWhite);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

QVector<RhiTextureData> CanvasLevelsNode::inputs() const
{
    return { m_input->value().value<RhiTextureData>() };
}

void CanvasLevelsNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    f[0] = float(m_inputBlack->value().toDouble());
    f[1] = float(m_inputWhite->value().toDouble());
    f[2] = float(m_gamma->value().toDouble());
    f[3] = 0.0f;
    f[4] = float(m_outputBlack->value().toDouble());
    f[5] = float(m_outputWhite->value().toDouble());
    f[6] = 0.0f;
    f[7] = 0.0f;
}

} // namespace photon
