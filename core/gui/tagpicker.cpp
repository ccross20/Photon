#include <QListWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include "tagpicker.h"
#include "photoncore.h"
#include "project/project.h"
#include "tag/tagcollection.h"

namespace photon {

class TagPicker::Impl{
public:
    QStringList tags;
    QLineEdit *tagEdit;
    QListWidget *list;
    QDialogButtonBox *buttons;
};

TagPicker::TagPicker(QWidget *parent)
    : QDialog{parent},m_impl(new Impl)
{
    auto vLayout = new QVBoxLayout;

    m_impl->list = new QListWidget;

    auto allTags = photonApp->project()->tags()->tags();
    m_impl->list->addItems(allTags);

    connect(m_impl->list, &QListWidget::itemClicked,this,[this](QListWidgetItem *item){
        m_impl->tagEdit->setText(m_impl->tagEdit->text() + " " + item->text());
    });

    vLayout->addWidget(m_impl->list);

    m_impl->tagEdit = new QLineEdit;


    vLayout->addWidget(m_impl->tagEdit);

    m_impl->buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                          | QDialogButtonBox::Cancel);

    connect(m_impl->buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_impl->buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    vLayout->addWidget(m_impl->buttons);

    connect(m_impl->tagEdit, &QLineEdit::textChanged,this,[this](const QString &text){
        m_impl->buttons->button(QDialogButtonBox::Ok)->setEnabled(text.length() > 0);
    });

    setLayout(vLayout);
}

TagPicker::~TagPicker()
{
    delete m_impl;
}

void TagPicker::accept()
{
    emit tagsPicked(m_impl->tagEdit->text().split(" "));

    QDialog::accept();
}

void TagPicker::reject()
{
    QDialog::reject();
}

} // namespace photon
