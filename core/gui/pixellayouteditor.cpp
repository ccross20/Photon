
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include "pixellayouteditor_p.h"
#include "pixel/pixelsourcelayout.h"
#include "pixel/pixellayout.h"
#include "pixel/pixelsource.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "photoncore.h"
#include "project/project.h"

namespace photon {

PixelLayoutEditorSidePanel::PixelLayoutEditorSidePanel(PixelLayout *t_layout) : QWidget(),pixelLayout(t_layout)
{
    addButton = new QPushButton("Add");
    layoutList = new QListWidget;

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(addButton);
    vLayout->addWidget(layoutList);

    setLayout(vLayout);

    connect(addButton, &QPushButton::clicked, this, &PixelLayoutEditorSidePanel::addClicked);
}

void PixelLayoutEditorSidePanel::addClicked()
{
    auto sources = SceneIterator::FindMany(photonApp->project()->sceneRoot(),[](SceneObject *obj, bool *keepGoing){
            *keepGoing = true;
       return dynamic_cast<PixelSource*>(obj);
    });


    auto currentSources = pixelLayout->sources();

    qDebug() << sources.length();

    QMenu menu;
    for(auto src : sources)
    {
        auto action = menu.addAction(src->name(),[src, this](){addSource(dynamic_cast<PixelSource*>(src));});
        action->setEnabled(!currentSources.contains(dynamic_cast<PixelSource*>(src)));
    }

    menu.exec(addButton->mapToGlobal(QPoint{}));

}

void PixelLayoutEditorSidePanel::addSource(photon::PixelSource *t_source)
{
    auto sourceLayout = new PixelSourceLayout(t_source);
    pixelLayout->addSource(sourceLayout);

    auto sourceObj = dynamic_cast<SceneObject*>(t_source);

    layoutList->addItem(sourceObj ? sourceObj->name() : "Unnamed");
}

PixelLayoutEditor::PixelLayoutEditor(PixelLayout *t_layout, QWidget *t_parent) : QWidget(t_parent),m_impl(new Impl)
{
    m_impl->sidePanel = new PixelLayoutEditorSidePanel(t_layout);


    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(m_impl->sidePanel);

    setLayout(hLayout);
}

PixelLayoutEditor::~PixelLayoutEditor()
{
    delete m_impl;
}

PixelLayout *PixelLayoutEditor::pixelLayout() const
{
    return m_impl->pixelLayout;
}

} // namespace photon
