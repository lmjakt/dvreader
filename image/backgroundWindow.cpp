#include "backgroundWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <iostream>


BackgroundWindow::BackgroundWindow(std::set<fluorInfo> fli, QWidget* parent)
  : QWidget(parent)
{

  QVBoxLayout* vbox = new QVBoxLayout(this);
  for(std::set<fluorInfo>::iterator it=fli.begin();
      it != fli.end(); it++){
    BackgroundWidget* bgw = new BackgroundWidget((*it), this);
    backgroundWidgets.insert(std::make_pair((*it), bgw));
    vbox->addWidget(bgw);
  }
  
  QPushButton* setParButton = new QPushButton("Set Parameters", this);
  connect(setParButton, SIGNAL(clicked()), this, SLOT(setPars()) );

  QHBoxLayout* hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  hbox->addStretch();
  hbox->addWidget(setParButton);
}

void BackgroundWindow::setPars(){
  std::map<fluorInfo, backgroundPars> flPars;
  for(std::map<fluorInfo, BackgroundWidget*>::iterator it=backgroundWidgets.begin();
      it != backgroundWidgets.end(); it++){
    flPars.insert(std::make_pair((*it).first, (*it).second->pars()));
  }
  for(std::map<fluorInfo, backgroundPars>::iterator it=flPars.begin(); it != flPars.end(); ++it){
    std::cout << it->second.x_m << "," << it->second.y_m << "," << it->second.z_m << " : " 
	      << it->second.pcntile << std::endl;
  }
  emit setBackgroundPars(flPars);
}

