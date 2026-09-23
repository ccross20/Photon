#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include "nodeeditor.h"
#include "model/node.h"
#include "model/parameter/parameter.h"
#include "propertywidgets.h"

namespace keira {

NodeEditor::NodeEditor(QWidget *parent)
    : QWidget{parent}
{
    m_vLayout = new QVBoxLayout;
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_vLayout);

    // Built on PropertyForm so a node's parameters are laid out identically to
    // a gizmo's properties and a resource's fields - all three now appear in
    // the same Properties panel, where any difference reads as a bug.
    m_form = new photon::PropertyForm;
    m_vLayout->addWidget(m_form);
}

void NodeEditor::setNode(Node *t_node)
{
    if(m_node == t_node)
        return;
    m_node = t_node;
    rebuildParameters();
}

Node *NodeEditor::node() const
{
    return m_node;
}

void NodeEditor::rebuildParameters()
{
    m_form->clear();

    if(!m_node)
    {
        m_form->addStretch();
        return;
    }

    QLineEdit *nameEdit = new QLineEdit;
    nameEdit->setMaximumHeight(30);
    nameEdit->setText(m_node->name());
    connect(nameEdit, &QLineEdit::textEdited, this, [this](const QString &name){
        m_node->setName(name);
    });
    m_form->addRow("Name", nameEdit);

    for(Parameter *param : m_node->parameters())
        m_form->addRow(param->name(), param->createWidget(this));

    // Optional custom editor UI, below the parameter rows. When present it takes
    // the remaining height (so e.g. FixtureStateNode's scrolling capability list
    // fills the panel); otherwise a stretch keeps the rows pinned to the top.
    if(QWidget *custom = m_node->createCustomWidget(this))
        m_form->addFullWidth(custom, 1);
    else
        m_form->addStretch();
}

void NodeEditor::widgetUpdated(QWidget *t_widget, const keira::Parameter *t_param)
{
    m_node->setValue(t_param->id(), t_param->updateValue(t_widget));
}

} // namespace keira
