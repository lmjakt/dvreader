#include "distPlotter.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <iostream>
#include <math.h>

using namespace std;

DistPlotter::DistPlotter(QWidget* parent)
    : QWidget(parent)
{ 
    min=max = 0;
    isLog = false;
    divNo = 100;
 
    linePlotter = new LinePlotter(this);
    connect(linePlotter, SIGNAL(doubleClicked()), this, SLOT(toggleLog()) );
    connect(linePlotter, SIGNAL(mousePos(int, float)), this, SLOT(displayPos(int, float)) );

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

void DistPlotter::setData(vector<vector<float> >& v, vector<QColor>& c, bool lg){

    cout << "DistPlotter::setData " << endl;
    values = v;
    colors = c;
    isLog = lg;

    initCount();
    float range = max - min;
    if(!range)
	return;

    countItems();


}

void DistPlotter::countItems(){
  float minCell = minValueBox->value();
  float maxCell = maxValueBox->value();
  float range = maxCell - minCell;
  
  if(range <= 0)
    return;

  for(uint i=0; i < values.size(); ++i){
    cout << i << " values size: " << values[i].size() << endl;
    for(uint j=0; j < values[i].size(); ++j){
      if(values[i][j] <= maxCell && values[i][j] >= minCell)
	counts[i][ (uint) roundf(  ((float)counts[i].size()-1) *  ((values[i][j] - minCell) / range ) ) ]++;
    }
  }
  linePlotter->setData(counts, colors);
}

void DistPlotter::initCount(){
    min = max = 0;
    if(!values.size() || !values[0].size()){
	cerr << "DistPlotter::initCount empty values" << endl;
	return;
    }
    min = max = values[0][0];
    counts.resize(values.size());
    for(uint i=0; i < values.size(); ++i){
	counts[i].resize(divNo);
	counts[i].assign(divNo, 0);
	for(uint j=0; j < values[i].size(); ++j){
	    min = min > values[i][j] ? values[i][j] : min;
	    max = max < values[i][j] ? values[i][j] : max;
	}
    }
    minValueBox->setRange(min, max);
    maxValueBox->setRange(min, max);
    minValueBox->setValue(min);
    maxValueBox->setValue(max);
}

void DistPlotter::setLog(bool l){
    if(l == isLog)
	return;
    isLog = l;
    if(isLog){
	setLogValues();
    }else{
	setLinValues();
    }
    setData(values, colors, isLog);
}

void DistPlotter::toggleLog(){
    setLog(!isLog);
}

void DistPlotter::setLogValues(){
    for(uint i=0; i < values.size(); ++i){
	for(uint j=0; j < values[i].size(); ++j)
	    values[i][j] = log(values[i][j]);
    }
}

void DistPlotter::setLinValues(){
    for(uint i=0; i < values.size(); ++i){
	for(uint j=0; j < values[i].size(); ++j)
	    values[i][j] = exp(values[i][j]);
    }
}

void DistPlotter::displayPos(int xp, float yp){
    QString xl,yl;
    yl.setNum(yp);
    
    float minCell = minValueBox->value();
    float maxCell = maxValueBox->value();

    float x = minCell + ((float)xp / (float)divNo) * (maxCell - minCell);
    if(isLog)
	x = exp(x);
//    cout << "Position " << xp << " --> " << x << "  max:min:divno " << max << " : " << min << " : " << divNo << endl;
    xl.setNum(x);
    xPos->setText(xl);
    yPos->setText(yl);
}

void DistPlotter::setCellNo(int dno){
  divNo = (unsigned int)dno;
  for(uint i=0; i < counts.size(); ++i){
    counts[i].resize(divNo);
    counts[i].assign(divNo, 0);
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
 
