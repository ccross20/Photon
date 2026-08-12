#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include "resourceeditorwidget.h"
#include "project/project.h"
#include "project/projectresource.h"
#include "gui/tag/tageditorwidget.h"
#include "photoncore.h"

namespace photon {

class ResourceEditorWidget::Impl
{
public:
    ProjectResource *resource = nullptr;
    QFormLayout *layout = nullptr;
    QLineEdit *nameEdit = nullptr;
    TagEditorWidget *tagEditor = nullptr;
    QPushButton *openButton = nullptr;
    // Guards the resource -> widget refresh so it doesn't loop back through
    // the editingFinished handlers.
    bool updating = false;
};

ResourceEditorWidget::ResourceEditorWidget(ProjectResource *t_resource, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl)
{
    m_impl->resource = t_resource;

    m_impl->layout = new QFormLayout(this);

    m_impl->nameEdit = new QLineEdit;

    m_impl->tagEditor = new TagEditorWidget(
        [this](){ return m_impl->resource ? m_impl->resource->resourceTags() : QStringList(); },
        [this](const QStringList &tags){ if(m_impl->resource) m_impl->resource->setResourceTags(tags); },
        [](){ return photonApp->project() ? photonApp->project()->allTags() : QStringList(); });

    m_impl->layout->addRow("Name", m_impl->nameEdit);
    m_impl->layout->addRow("Tags", m_impl->tagEditor);

    connect(m_impl->nameEdit, &QLineEdit::editingFinished, this, &ResourceEditorWidget::nameEdited);

    if(m_impl->resource)
    {
        connect(m_impl->resource->resourceNotifier(), &ProjectResourceNotifier::resourceChanged,
                this, &ResourceEditorWidget::resourceChanged);
        // The Properties panel keeps showing this widget until the selection
        // changes, which can outlive the resource itself (deleted from the
        // project panel).
        connect(m_impl->resource->resourceObject(), &QObject::destroyed,
                this, &ResourceEditorWidget::resourceDestroyed);
    }

    resourceChanged();
}

void ResourceEditorWidget::setOpenAction(const QString &t_label, std::function<void()> t_callback)
{
    if(!m_impl->openButton)
    {
        m_impl->openButton = new QPushButton;
        m_impl->layout->addRow(m_impl->openButton);
    }

    m_impl->openButton->setText(t_label);
    connect(m_impl->openButton, &QPushButton::clicked, this, [this, t_callback](){
        // Nothing to open once the resource is gone.
        if(m_impl->resource)
            t_callback();
    });
}

void ResourceEditorWidget::resourceDestroyed()
{
    m_impl->resource = nullptr;
    m_impl->nameEdit->setEnabled(false);
    m_impl->tagEditor->setEnabled(false);
    if(m_impl->openButton)
        m_impl->openButton->setEnabled(false);
}

ResourceEditorWidget::~ResourceEditorWidget()
{
    delete m_impl;
}

void ResourceEditorWidget::resourceChanged()
{
    if(!m_impl->resource)
        return;

    m_impl->updating = true;
    m_impl->nameEdit->setText(m_impl->resource->resourceName());
    m_impl->updating = false;

    m_impl->tagEditor->refresh();
}

void ResourceEditorWidget::nameEdited()
{
    if(!m_impl->resource || m_impl->updating)
        return;

    const QString name = m_impl->nameEdit->text();
    if(name == m_impl->resource->resourceName())
        return;

    m_impl->resource->setResourceName(name);
}

} // namespace photon
