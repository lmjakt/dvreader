#include "cavityBallInputWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

CavityBallInputWidget::CavityBallInputWidget(QWidget* parent) :
  QWidget(parent)
{
  chSelector = new ChannelSelector(this);
  
  xRadius = makeSpinBox("xr ", 2, 20);
  yRadius = makeSpinBox("yr ", 2, 20);
  zRadius = makeSpinBox("zr ", 2, 20);

  maxP = makeDoubleSpinBox("P ", 0, 2.0, 0.001);
  maxDP = makeDoubleSpinBox("DP ", 0, 200, 0.1);

  chSelector = new ChannelSelector(this);
  
  QPushButton* mapButton = new QPushButton("map Cavities", this);
  connect(mapButton, SIGNAL(clicked()), this, SLOT(mapCavities()) );

  QVBoxLayout* vbox = new QVBoxLayout(this);
  vbox->addWidget(chSelector);
  QHBoxLayout* hbox = new QHBoxLayout();
  vbox->addLayout(hbox);
  hbox->addWidget(xRadius);
  hbox->addWidget(yRadius);
  hbox->addWidget(zRadius);
  hbox->addSpacing(10);
  hbox->addWidget(maxP);
  hbox->addWidget(maxDP);
  hbox->addSpacing(15);
  hbox->addWidget(mapButton);
}

void CavityBallInputWidget::setChannels(std::vector<QString> channels){
  chSelector->setChannels(channels);
}

void CavityBallInputWidget::mapCavities(){
  int xr = xRadius->value();
  int yr = yRadius->value();
  int zr = zRadius->value();
  float P = (float)maxP->value();
  float DP = (float)maxDP->value();
  int channel = chSelector->selectedId();
  
  if(channel == -1)
    return;
  emit mapCavities(channel, xr, yr, zr, P, DP);
}

 
QSpinBox* CavityBallInputWidget::makeSpinBox(QString pre, int min, int max){
  QSpinBox* box = new QSpinBox(this);
  box->setPrefix(pre);
  box->setMinimum(min);
  box->setMaximum(max);
  return(box);
}

QDoubleSpinBox* CavityBallInputWidget::makeDoubleSpinBox(QString pre, float min, float max, float step){
  QDoubleSpinBox* box = new QDoubleSpinBox(this);
  box->setPrefix(pre);
  box->setMinimum(min);
  box->setMaximum(max);
  box->setSingleStep(step);
  return(box);
}

