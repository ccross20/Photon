#include <QLabel>
#include <QLineEdit>
#include "nodeeditor.h"
#include "model/node.h"
#include "model/parameter/parameter.h"

namespace keira {

NodeEditor::NodeEditor(QWidget *parent)
    : QWidget{parent}
{
    m_gridLayout = new QGridLayout;
    m_vLayout = new QVBoxLayout;

    setLayout(m_vLayout);
    setMinimumWidth(300);
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
    removeAllFromLayout(m_vLayout);

    if(m_node)
    {
        m_gridLayout->addWidget(new QLabel("Name"),0,0);

        QLineEdit *nameEdit = new QLineEdit();
        nameEdit->setText(m_node->name());
        connect(nameEdit, &QLineEdit::textEdited,this, [this](const QString &name){
            m_node->setName(name);
        });

        m_gridLayout->addWidget(nameEdit,0,1);



        int row = 1;
        for(Parameter *param : m_node->parameters())
        {
            QWidget *w = param->createWidget(this);

            m_gridLayout->addWidget(new QLabel(param->name()),row,0);
            m_gridLayout->addWidget(w,row,1);

            row++;
        }
    }


    m_vLayout->addLayout(m_gridLayout);
    m_vLayout->addStretch();

}

void NodeEditor::widgetUpdated(QWidget *t_widget, const keira::Parameter *t_param)
{
    m_node->setValue(t_param->id(), t_param->updateValue(t_widget));
}

void NodeEditor::removeAllFromLayout(QLayout *layout)
{
    while (layout->count()>0) {
        auto item = layout->takeAt(0);
        delete item->widget();
        if(item->layout())
            removeAllFromLayout(item->layout());
    }
}

} // namespace keira
