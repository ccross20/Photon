#include "propertiespanel_p.h"
#include "project/project.h"
#include "scene/sceneobject.h"

namespace photon {

PropertiesPanel::PropertiesPanel() : Panel("photon.properties"), m_impl(new Impl)
{
    setName("Properties");

    m_impl->layout = new QVBoxLayout;

    m_impl->emptyLabel = new QLabel("No selection");
    m_impl->emptyLabel->setAlignment(Qt::AlignCenter);
    m_impl->emptyLabel->setStyleSheet("color: #888;");
    m_impl->layout->addWidget(m_impl->emptyLabel);
    m_impl->layout->addStretch(1);

    setPanelLayout(m_impl->layout);
}

PropertiesPanel::~PropertiesPanel()
{
    delete m_impl;
}

void PropertiesPanel::selectedObjectChanged(SceneObject *obj)
{
    if(m_impl->editorWidget)
    {
        m_impl->layout->removeWidget(m_impl->editorWidget);
        delete m_impl->editorWidget;
        m_impl->editorWidget = nullptr;
    }

    m_impl->emptyLabel->setVisible(!obj);

    if(obj)
    {
        m_impl->editorWidget = obj->createEditor();
        m_impl->layout->insertWidget(0, m_impl->editorWidget);
    }
}

void PropertiesPanel::projectDidOpen(Project *project)
{
    connect(project, &Project::selectedSceneObjectChanged, this, &PropertiesPanel::selectedObjectChanged);
    selectedObjectChanged(project->selectedSceneObject());
}

void PropertiesPanel::projectWillClose(Project *project)
{
    disconnect(project, &Project::selectedSceneObjectChanged, this, &PropertiesPanel::selectedObjectChanged);
    selectedObjectChanged(nullptr);
}

} // namespace photon
