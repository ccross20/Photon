#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "channelparameterwidget.h"
#include "../channelparameter.h"
#include "channelparameterview.h"

namespace photon {

ChannelParameterWidget::ChannelParameterWidget(QVector<ChannelParameter*> t_parameters, QWidget *parent)
    : QWidget{parent}
{
    QGridLayout *grid = new QGridLayout;

    int row = 0;
    for(auto param : t_parameters)
    {
        grid->addWidget(new QLabel(param->name()),row,0);
        grid->addWidget(param->createView(),row,1);

        QPushButton *channelButton = new QPushButton("Add Channel");
        grid->addWidget(channelButton,row,2);

        connect(channelButton, &QPushButton::clicked,this,[this,param](){
            emit addChannel(param);
        });

        ++row;
    }
    grid->addItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding),row,0,1,3);

    setLayout(grid);
}

} // namespace photon
