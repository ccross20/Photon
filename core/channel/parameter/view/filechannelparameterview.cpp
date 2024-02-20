#include <QLineEdit>
#include <QSettings>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include "filechannelparameterview.h"
#include "../fileparameter.h"

namespace photon {
class FileChannelParameterView::Impl
{
public:
    QLineEdit *lineEdit;
};


FileChannelParameterView::FileChannelParameterView(FileParameter *t_param): ChannelParameterView(t_param),m_impl(new Impl)
{
    QHBoxLayout *hLayout = new QHBoxLayout;

    m_impl->lineEdit = new QLineEdit;
    m_impl->lineEdit->setReadOnly(t_param->isReadOnly());
    m_impl->lineEdit->setText(t_param->value().toString());
    hLayout->addWidget(m_impl->lineEdit);

    QPushButton *browseButton = new QPushButton("Browse");
    browseButton->setSizePolicy(QSizePolicy{QSizePolicy::Maximum,QSizePolicy::Maximum});
    hLayout->addWidget(browseButton);
    connect(browseButton, &QPushButton::clicked,this, &FileChannelParameterView::clickBrowse);
    connect(m_impl->lineEdit, &QLineEdit::textEdited,this, &FileChannelParameterView::pathUpdateSlot);

    setLayout(hLayout);
}

FileChannelParameterView::~FileChannelParameterView()
{
    delete m_impl;
}

void FileChannelParameterView::pathUpdateSlot(const QString &t_path)
{
    m_impl->lineEdit->setText(t_path);
    parameter()->setValue(t_path);

    emit pathUpdated(t_path);
}

void FileChannelParameterView::clickBrowse()
{
    QSettings qsettings;
    qsettings.beginGroup("app");
    QString startPath = qsettings.value("loadpath", QDir::homePath()).toString();
    qsettings.endGroup();

    QString path = QFileDialog::getOpenFileName(nullptr, "Photon Project",
                                            startPath,
                                            "*.jpg *.jpeg *.png *.gif *.bpm");

    if(path.isNull())
        return;
    qsettings.beginGroup("app");
    qsettings.setValue("loadpath", QFileInfo(path).path());
    qsettings.endGroup();

    pathUpdateSlot(path);
}

} // namespace photon
