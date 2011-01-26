#include "distPlotter.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <iostream>
#include <math.h>

using namespace std;

DistPlotter::DistPlotter(bool useLimits, QWidget* parent)
    : QWidget(parent)
{ 
    min=max = 0;
    isLog = false;
    divNo = 100;
 
    linePlotter = new LinePlotter(this);
    linePlotter->enableLimits(useLimits);
    connect(linePlotter, SIGNAL(doubleClicked()), this, SLOT(toggleLog()) );
    connect(linePlotter, SIGNAL(mousePos(int, float)), this, SLOT(displayPos(int, float)) );
    if(useLimits){
      connect(linePlotter, SIGNAL(ctl_left(int, float)), this, SLOT(setLeftLimit(int, float)) );
      connect(linePlotter, SIGNAL(ctl_right(int, float)), this, SLOT(setRightLimit(int, float)) );
    }

    xPos = new QLabel("NA", this);
    yPos = new QLabel("NA", this);

    xPos->setMinimumWidth(40);
    yPos->setMinimumWidth(40);

    minValueBox = new QDoubleSpinBox(this);
    maxValueBox = new QDoubleSpinBox(this);
    cellNoBox = new QSpinBox(this);
    cellNoBox->setRange(1, 200);
    cellNoBox->setValue(divNo);

    QLabel* minBoxLabel = new QLabel("min", this);
    QLabel* maxBoxLabel = new QLabel("max", this);
    QLabel* cellNoLabel = new QLabel("cell#", this);

    connect(minValueBox, SIGNAL(valueChanged(double)), this, SLOT(setMinCell(double)) );
    connect(maxValueBox, SIGNAL(valueChanged(double)), this, SLOT(setMaxCell(double)) );
    connect(cellNoBox, SIGNAL(valueChanged(int)), this, SLOT(setCellNo(int)) );

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->addWidget(linePlotter);
    vbox->setStretchFactor(linePlotter, 10);

    QHBoxLayout* hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    hbox->addWidget(cellNoLabel);
    hbox->addWidget(cellNoBox);
    hbox->addWidget(minBoxLabel);
    hbox->addWidget(minValueBox);
    hbox->addWidget(maxBoxLabel);
    hbox->addWidget(maxValueBox);
    hbox->addStretch();
    hbox->addWidget(xPos);
    hbox->addWidget(yPos);
}

DistPlotter::~DistPlotter(){
}

void DistPlotter::setSingleData(std::vector<float> v, std::vector<QColor>& c, bool resetLimits){
  vector<vector<float> > vv;
  vv.push_back(v);
  setData(vv, c, resetLimits);
}

void DistPlotter::setSingleData(std::vector<float> v, bool resetLimits){
  vector<vector<float> > vv;
  vv.push_back(v);
  setData(vv, resetLimits);
}

void DistPlotter::setData(vector<vector<float> >& v, vector<QColor>& c, bool resetLimits){
  colors = c;
  setData(v, resetLimits);
}

void DistPlotter::setData(vector<vector<float> >& v, bool resetLimits){
    values = v;
    //    isLog = lg;
    setLogValues();

    initCount(resetLimits);
    float range = max - min;
    if(!range){
      counts.resize(0);
      colors.resize(0);
      linePlotter->setData(counts, colors);
      return;
    }
    countItems();
}


void DistPlotter::initCount(bool resetLimits){
    min = max = 0;
    //    vector<vector<float> >& v = isLog ? logValues : values;
    vector<vector<float> >& v = values;
    if(!v.size() || !v[0].size()){
      cerr << "DistPlotter::initCount empty v" << endl;
      return;
    }
    min = max = v[0][0];
    counts.resize(v.size());
    for(uint i=0; i < v.size(); ++i){
      counts[i].resize(divNo);
      //      counts[i].assign(divNo, 0);
      for(uint j=0; j < v[i].size(); ++j){
	min = min > v[i][j] ? v[i][j] : min;
	max = max < v[i][j] ? v[i][j] : max;
      }
    }
    minValueBox->blockSignals(true);
    maxValueBox->blockSignals(true);
    minValueBox->setRange(min, max);
    maxValueBox->setRange(min, max);
    minValueBox->setValue(min);
    maxValueBox->setValue(max);
    minValueBox->blockSignals(false);
    maxValueBox->blockSignals(false);
    if(resetLimits){
      leftLimit = min;
      rightLimit = max;
    }
}

void DistPlotter::countItems(){
  float minCell = isLog ? log(minValueBox->value()) : minValueBox->value();
  float maxCell = isLog ? log(maxValueBox->value()) : maxValueBox->value();
  float range = maxCell - minCell;
  
  if(range <= 0)
    return;

  vector<vector<float> >& v = isLog ? logValues : values;
  zero_counts();
  for(uint i=0; i < v.size(); ++i){
    for(uint j=0; j < v[i].size(); ++j){
      if(v[i][j] <= maxCell && v[i][j] >= minCell)
	counts[i][ (uint) roundf(  ((float)counts[i].size()-1) *  ((v[i][j] - minCell) / range ) ) ]++;
    }
  }

  linePlotter->setMasks(cell_pos(leftLimit), 1 + cell_pos(rightLimit) );
  linePlotter->setData(counts, colors, false);
}


void DistPlotter::setLog(bool l){
    if(l == isLog)
	return;
    isLog = l;
    initCount(false);
    countItems();
}

void DistPlotter::toggleLog(){
    setLog(!isLog);
}

void DistPlotter::setLogValues(){
  logValues.resize(values.size());
    for(uint i=0; i < values.size(); ++i){
      logValues[i].resize(values[i].size());
	for(uint j=0; j < values[i].size(); ++j)
	    logValues[i][j] = log(values[i][j]);
    }
}


void DistPlotter::displayPos(int xp, float yp){
    QString xl,yl;
    yl.setNum(yp);
    
    float x = translate_xpos(xp);
    xl.setNum(x);
    xPos->setText(xl);
    yPos->setText(yl);
}

void DistPlotter::setPlotLimits(float l, float r, bool updatePlot){
  leftLimit = l;
  rightLimit = r;
  cout << "DistPlotter::setPlotLimits " << l << " : " << r << "  update? " << updatePlot << endl;
  if(updatePlot)
    linePlotter->setMasks(cell_pos(leftLimit), 1 + cell_pos(rightLimit) );
}

void DistPlotter::setCellNo(int dno){
  divNo = (unsigned int)dno;
  for(uint i=0; i < counts.size(); ++i){
    counts[i].resize(divNo);
    //    counts[i].assign(divNo, 0);
  }
  countItems();
}

void DistPlotter::setMinCell(double v){
  v = v;
  countItems();
}

void DistPlotter::setMaxCell(double v){
  v = v;
  countItems();
}
 
void DistPlotter::setLeftLimit(int x, float y){
  y = y;
  leftLimit = translate_xpos(x);
  linePlotter->setLeftMask(cell_pos(leftLimit));
  emit setLimits(leftLimit, rightLimit);
}

void DistPlotter::setRightLimit(int x, float y){
  y = y;
  rightLimit = translate_xpos(x);
  linePlotter->setRightMask(1 + cell_pos(rightLimit));
  emit setLimits(leftLimit, rightLimit);
}

float DistPlotter::translate_xpos(int x){
  float minCell = minValueBox->value();
  float maxCell = maxValueBox->value();  
  if(isLog){
    minCell = log(minCell);
    maxCell = log(maxCell);
  }
  float xp = minCell + ((float)x / (float)divNo) * (maxCell - minCell);
  if(isLog)
    xp = exp(xp);
  return(xp);
}
	 
unsigned int DistPlotter::cell_pos(float v){
  v = isLog ? log(v) : v;
  float minCell = isLog ? log(minValueBox->value()) : minValueBox->value();
  float maxCell = isLog ? log(maxValueBox->value()) : maxValueBox->value();
  float range = maxCell - minCell;

  return( (uint) roundf(  ((float)divNo - 1) *  ((v - minCell) / range ) ) );
}

void DistPlotter::zero_counts(){
  for(uint i=0; i < counts.size(); ++i)
    counts[i].assign(counts[i].size(), 0);
}
