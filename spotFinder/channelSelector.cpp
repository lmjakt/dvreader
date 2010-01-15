#include "channelSelector.h"
#include <QRadioButton>
#include <QList>


ChannelSelector::ChannelSelector(QWidget* parent) :
  QWidget(parent)
{
  channelGroup = new QButtonGroup(this);
  hbox = new QHBoxLayout(this);
}

void ChannelSelector::setChannels(std::vector<QString> channels){
  QList<QAbstractButton*> oldButtons = channelGroup->buttons();
  for(int i=0; i < oldButtons.size(); ++i){
    channelGroup->remove(oldButtons[i]);
    delete(oldButtons[i]);
  }
  for(uint i=0; i < channels.size(); ++i){
    QRadioButton* button = new QRadioButton(channels[i]);
    channelGroup->addButton(button, i);
    hbox->addWidget(button);
    button->show();
  }
}

int ChannelSelector::selectedId(){
  return(channelGroup->checkedId());
}


  
