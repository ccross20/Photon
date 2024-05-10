#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include "channelparameterwidget.h"
#include "../channelparameter.h"
#include "channelparameterview.h"

namespace photon {

ChannelParameterWidget::ChannelParameterWidget(QVector<ChannelParameter*> t_parameters, std::function<bool(ChannelParameter *)> allow, QWidget *parent)
    : QWidget{parent}
{
    QGridLayout *grid = new QGridLayout;

    int row = 0;
    for(auto param : t_parameters)
    {
        grid->addWidget(new QLabel(param->name()),row,0);
        grid->addWidget(param->createView(),row,1);

        QPushButton *channelButton = new QPushButton("Add Channel");
        channelButton->setEnabled(allow(param));
        grid->addWidget(channelButton,row,2);

        if(param->type() == ChannelParameter::ChannelParameterColor)
        {
            connect(channelButton, &QPushButton::clicked,this,[this,param,channelButton](){
                QMenu menu;
                menu.addAction("Numbers",[this, param,channelButton](){
                    channelButton->setEnabled(false);
                    emit addChannel(param, ChannelInfo::ChannelTypeNumber);
                });
                menu.addAction("Color",[this, param,channelButton](){
                    channelButton->setEnabled(false);
                    emit addChannel(param, ChannelInfo::ChannelTypeColor);
                });

                menu.exec(channelButton->mapToGlobal(QPoint{}));
            });


        }
        else if(param->type() == ChannelParameter::ChannelParameterPoint)
        {
            connect(channelButton, &QPushButton::clicked,this,[this,param,channelButton](){
                channelButton->setEnabled(false);
                emit addChannel(param, ChannelInfo::ChannelTypePoint);
            });
        }
        else
        {
            connect(channelButton, &QPushButton::clicked,this,[this,param,channelButton](){
                channelButton->setEnabled(false);
                emit addChannel(param, ChannelInfo::ChannelTypeNumber);
            });
        }



        ++row;
    }
    grid->addItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::Expanding),row,0,1,3);

    setLayout(grid);
}

} // namespace photon
