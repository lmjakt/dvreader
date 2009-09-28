#include "blobScatterPlot.h"
#include <iostream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
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

  xParam->setCurrentIndex(BlobMapper::VOLUME);
  yParam->setCurrentIndex(BlobMapper::SUM);

  connect(xParam, SIGNAL(activated(int)), this, SLOT(changePlotParams(int)) );
  connect(yParam, SIGNAL(activated(int)), this, SLOT(changePlotParams(int)) );

  QLabel* xLabel = new QLabel("X", this);
  QLabel* yLabel = new QLabel("Y", this);

  alphaBox = new QSpinBox(this);
  alphaBox->setRange(0, 255);
  alphaBox->setValue(255);
  connect(alphaBox, SIGNAL(valueChanged(int)), plotter, SLOT(setAlpha(int)) );

  QPushButton* filterButton = new QPushButton("filter", this);
  QPushButton* selectButton = new QPushButton("select", this);
  connect(filterButton, SIGNAL(clicked()), this, SLOT(filterBlobs()) );
  connect(selectButton, SIGNAL(clicked()), this, SLOT(selectBlobs()) );

  QVBoxLayout* mainBox = new QVBoxLayout(this);
  mainBox->addWidget(plotter);
  hbox = new QHBoxLayout();
  mainBox->addLayout(hbox);
  hbox->addWidget(xLabel);
  hbox->addWidget(xParam);
  hbox->addWidget(yLabel);
  hbox->addWidget(yParam);
  hbox->addWidget(alphaBox);
  hbox->addWidget(filterButton);
  hbox->addWidget(selectButton);
  hbox->addStretch();
}

void BlobScatterPlot::setData(vector<vector<float> > xv, vector<vector<float> > yv, vector<QColor> c)
{
  cout << "BlobScatterPlot::setData() " << endl;
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

BlobMapper::Param BlobScatterPlot::x_param(){
  return(getParam(xParam));
}

BlobMapper::Param BlobScatterPlot::y_param(){
  return(getParam(yParam));
}

void BlobScatterPlot::changePlotParams(int p){
  p = p;
  BlobMapper::Param xp = getParam(xParam);
  BlobMapper::Param yp = getParam(yParam);
  if(xParam < 0 || yParam < 0){
    cerr << "BlobScatterPlot::changePlotParams unknown param : " << xParam << " : " << yParam << endl;
    return;
  }
  cout << "blobscatterplot params requested : " << xp << " , " << yp << endl;
  emit plotPars(xp, yp);
}

void BlobScatterPlot::changeSelection(){
  cout << "BlobScatterPlot::changeSelection" << endl;
  vector<bool> b(colorBoxes.size());
  for(uint i=0; i < colorBoxes.size(); ++i)
    b[i] = colorBoxes[i]->isChecked();
  plotter->setSelection(b);
}

void BlobScatterPlot::filterBlobs(){
    cout << "BlobScatterPlot::filterBlobs" << endl;
    vector<vector<bool> > selected = plotter->selectPoints(true);
    emit blobsSelected(selected);
}

void BlobScatterPlot::selectBlobs(){
    cout << "BlobScatterPlot::selectBlobs" << endl;
    vector<vector<bool > > selected = plotter->selectPoints(false);
    emit blobsSelected(selected);
}

void BlobScatterPlot::setParams(QComboBox* box){
  box->insertItem(BlobMapper::VOLUME, "Volume");
  box->insertItem(BlobMapper::SUM, "Sum");
  box->insertItem(BlobMapper::MEAN, "Mean");
  box->insertItem(BlobMapper::MAX, "Max");
  box->insertItem(BlobMapper::MIN, "Min");
  box->insertItem(BlobMapper::EXTENT, "Extent");
  box->insertItem(BlobMapper::SURFACE, "Surface");
  box->insertItem(BlobMapper::BACKGROUND, "Background");
  box->insertItem(BlobMapper::ASUM, "Adj. Sum");
}

BlobMapper::Param BlobScatterPlot::getParam(QComboBox* box){
  BlobMapper::Param p = BlobMapper::VOLUME;

  switch(box->currentIndex()){
  case BlobMapper::VOLUME:
    p = BlobMapper::VOLUME;
    break;
  case BlobMapper::SUM:
    p = BlobMapper::SUM;
    break;
  case BlobMapper::MEAN:
    p = BlobMapper::MEAN;
    break;
  case BlobMapper::MAX:
    p = BlobMapper::MAX;
    break;
  case BlobMapper::MIN:
    p = BlobMapper::MIN;
    break;
  case BlobMapper::EXTENT:
    p = BlobMapper::EXTENT;
    break;
  case BlobMapper::SURFACE:
    p = BlobMapper::SURFACE;
    break;
  case BlobMapper::BACKGROUND:
    p = BlobMapper::BACKGROUND;
    break;
  case BlobMapper::ASUM:
    p = BlobMapper::ASUM;
    break;
  default:
    p = BlobMapper::SUM;
  }
  return(p);
}
    
