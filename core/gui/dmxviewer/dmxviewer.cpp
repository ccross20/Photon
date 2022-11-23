#include <QVBoxLayout>
#include <QTimer>
#include "dmxviewer.h"
#include "dmxviewercells.h"
#include "data/dmxmatrix.h"

namespace photon {

class DMXViewer::Impl
{
public:
    QTimer timer;
    DMXViewerCells *cells;
    int counter = 0;
};

DMXViewer::DMXViewer(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->cells = new DMXViewerCells;

    QVBoxLayout *vLayout = new QVBoxLayout;

    vLayout->addWidget(m_impl->cells);
    setLayout(vLayout);

    setStyleSheet("background:#363636;");


    connect(&m_impl->timer, &QTimer::timeout, this, &DMXViewer::timeout);
    m_impl->timer.start(1000/24);
}

DMXViewer::~DMXViewer()
{
    delete m_impl;
}

void DMXViewer::timeout()
{
    DMXMatrix matrix;


    int index = m_impl->counter;
    int channel = 0;
    for(auto it = matrix.channels[0].begin(); it != matrix.channels[0].end(); ++it)
    {
        *it = (channel + index);
        ++channel;
    }
    m_impl->cells->setData(matrix);

    ++m_impl->counter;
}


} // namespace photon
