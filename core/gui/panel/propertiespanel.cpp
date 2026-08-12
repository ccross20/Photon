#include "propertiespanel_p.h"
#include "project/project.h"
#include "project/projectresource.h"
#include "photoncore.h"

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

// A resource was selected — route its editor through the shared properties
// widget so any selection source (project panel, viewport, channel effect, ...)
// uses one path. Works for every resource type, not just scene objects.
void PropertiesPanel::selectedResourceChanged(ProjectResource *resource)
{
    photonApp->project()->setPropertiesWidget(resource ? resource->createResourceEditor() : nullptr);
}

// Display the current properties widget. The Project owns it (and deletes the
// previous one), so we only manage layout membership — never delete here.
void PropertiesPanel::propertiesWidgetChanged(QWidget *widget)
{
    if(m_impl->editorWidget)
    {
        m_impl->layout->removeWidget(m_impl->editorWidget);
        m_impl->editorWidget->setParent(nullptr);
        m_impl->editorWidget = nullptr;
    }

    m_impl->emptyLabel->setVisible(!widget);

    if(widget)
    {
        m_impl->layout->insertWidget(0, widget);
        widget->show();
        m_impl->editorWidget = widget;
    }
}

void PropertiesPanel::projectDidOpen(Project *project)
{
    connect(project, &Project::selectedResourceChanged, this, &PropertiesPanel::selectedResourceChanged);
    connect(project, &Project::propertiesWidgetChanged, this, &PropertiesPanel::propertiesWidgetChanged);

    propertiesWidgetChanged(project->propertiesWidget());
    selectedResourceChanged(project->selectedResource());
}

void PropertiesPanel::projectWillClose(Project *project)
{
    disconnect(project, &Project::selectedResourceChanged, this, &PropertiesPanel::selectedResourceChanged);
    disconnect(project, &Project::propertiesWidgetChanged, this, &PropertiesPanel::propertiesWidgetChanged);

    // Detach our view of the widget, then clear it in the project (which deletes it).
    propertiesWidgetChanged(nullptr);
    project->setPropertiesWidget(nullptr);
}

} // namespace photon
