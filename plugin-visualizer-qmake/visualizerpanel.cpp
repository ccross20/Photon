
#include <QPainter>
#include <QImage>
#include <QQuickItem>

#include "visualizerpanel.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixturecollection.h"
#include "graph/bus/busevaluator.h"
#include "data/dmxmatrix.h"

namespace photon {

VisualizerPanel::VisualizerPanel(): Panel("Visualizer")
{

    m_viewport = new QQuickWidget;
    m_viewport->setMinimumSize(300, 300);
    m_viewport->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_viewport->setSource(QUrl("qrc:/main.qml"));

    auto root = m_viewport->rootObject();



    setPanelWidget(m_viewport);



/*


    for(auto fixture : photonApp->project()->fixtures()->fixtures())
    {

    }
    */

    connect(photonApp->busEvaluator(), &BusEvaluator::evaluationCompleted, this, &VisualizerPanel::dmxUpdated);

}

VisualizerPanel::~VisualizerPanel()
{

}

void VisualizerPanel::dmxUpdated()
{
    /*
    auto dmx = photonApp->busEvaluator()->dmxMatrix();
    for(auto model : m_models)
    {
        //model->updateFromDMX(dmx);
    */
}

} // namespace photon
