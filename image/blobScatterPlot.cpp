#include "blobScatterPlot.h"
#include <iostream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>


using namespace std;

BlobScatterPlot::BlobScatterPlot(QWidget* parent)
  : QWidget(parent)
{
  plotter = new ScatterPlotter(this);
  xParam = new QComboBox(this);
  yParam = new QComboBox(this);
  setParams(xParam);
  setParams(yParam);

  xParam->setCurrentIndex(BlobMapperWidget::VOLUME);
  yParam->setCurrentIndex(BlobMapperWidget::SUM);

  connect(xParam, SIGNAL(activated(int)), this, SLOT(changePlotParams(int)) );
  connect(yParam, SIGNAL(activated(int)), this, SLOT(changePlotParams(int)) );

  QLabel* xLabel = new QLabel("X", this);
  QLabel* yLabel = new QLabel("Y", this);

  alphaBox = new QSpinBox(this);
  alphaBox->setRange(0, 255);
  alphaBox->setValue(255);
  connect(alphaBox, SIGNAL(valueChanged(int)), plotter, SLOT(setAlpha(int)) );

  QVBoxLayout* mainBox = new QVBoxLayout(this);
  mainBox->addWidget(plotter);
  hbox = new QHBoxLayout();
  mainBox->addLayout(hbox);
  hbox->addWidget(xLabel);
  hbox->addWidget(xParam);
  hbox->addWidget(yLabel);
  hbox->addWidget(yParam);
  hbox->addWidget(alphaBox);
  hbox->addStretch();
}

void BlobScatterPlot::setData(vector<vector<float> > xv, vector<vector<float> > yv, vector<QColor> c)
{
  for(uint i=0; i < c.size(); ++i)
    c[i].setAlpha(alphaBox->value());

  for(uint i=0; i < colorBoxes.size(); ++i)
    delete colorBoxes[i];
  
  colorBoxes.clear();

  QPalette pal = palette();
  for(uint i=0; i < c.size(); ++i){
    colorBoxes.push_back(new QCheckBox(this));
    pal.setColor(QPalette::Button, c[i]);
    colorBoxes.back()->setPalette(pal);
    colorBoxes.back()->setChecked(true);
    connect(colorBoxes.back(), SIGNAL(clicked()), this, SLOT(changeSelection()) );
    hbox->insertWidget(-1, colorBoxes.back());
    colorBoxes.back()->show();
  }
  plotter->setData(xv, yv, c);
}

BlobMapperWidget::Param BlobScatterPlot::x_param(){
  return(getParam(xParam));
}

BlobMapperWidget::Param BlobScatterPlot::y_param(){
  return(getParam(yParam));
}

void BlobScatterPlot::changePlotParams(int p){
  p = p;
  BlobMapperWidget::Param xp = getParam(xParam);
  BlobMapperWidget::Param yp = getParam(yParam);
  if(xParam < 0 || yParam < 0){
    cerr << "BlobScatterPlot::changePlotParams unknown param : " << xParam << " : " << yParam << endl;
    return;
  }
  cout << "blobscatterplot params requested : " << xp << " , " << yp << endl;
  emit plotPars(xp, yp);
}

void BlobScatterPlot::changeSelection(){
  vector<bool> b(colorBoxes.size());
  for(uint i=0; i < colorBoxes.size(); ++i)
    b[i] = colorBoxes[i]->isChecked();
  plotter->setSelection(b);
}

void BlobScatterPlot::setParams(QComboBox* box){
  box->insertItem(BlobMapperWidget::VOLUME, "Volume");
  box->insertItem(BlobMapperWidget::SUM, "Sum");
  box->insertItem(BlobMapperWidget::MEAN, "Mean");
  box->insertItem(BlobMapperWidget::MAX, "Max");
  box->insertItem(BlobMapperWidget::MIN, "Min");
  box->insertItem(BlobMapperWidget::EXTENT, "Extent");
}

BlobMapperWidget::Param BlobScatterPlot::getParam(QComboBox* box){
  BlobMapperWidget::Param p = BlobMapperWidget::VOLUME;

  switch(box->currentIndex()){
  case BlobMapperWidget::VOLUME:
    p = BlobMapperWidget::VOLUME;
    break;
  case BlobMapperWidget::SUM:
    p = BlobMapperWidget::SUM;
    break;
  case BlobMapperWidget::MEAN:
    p = BlobMapperWidget::MEAN;
    break;
  case BlobMapperWidget::MAX:
    p = BlobMapperWidget::MAX;
    break;
  case BlobMapperWidget::MIN:
    p = BlobMapperWidget::MIN;
    break;
  case BlobMapperWidget::EXTENT:
    p = BlobMapperWidget::EXTENT;
    break;
  default:
    p = BlobMapperWidget::SUM;
  }
  return(p);
}
    
