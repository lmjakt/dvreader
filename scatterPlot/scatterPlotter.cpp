#include "scatterPlotter.h"
#include <QPainter>
#include <iostream>
#include <math.h>

using namespace std;

ScatterPlotter::ScatterPlotter(QWidget* parent)
  : QWidget(parent)
{
  plotLog = false;
  vMargin = 10;
  hMargin = 10;
  tick_spacing = 30;
  tick_length = 5;
  xScale = 1.0;
  yScale = 1.0;
  m_size = 6;
  setFocusPolicy(Qt::StrongFocus);
}

bool ScatterPlotter::setData(vector<vector<float> > xv, vector<vector<float> > yv, vector<QColor> c)
{
  if(xv.size() != yv.size() || xv.size() != c.size()){
    cerr << "ScatterPlotter::setData null or differently sized data vectors "
	 << xv.size() << "\t" << yv.size() << " : " << c.size() << endl;
    return(false);
  }
  lin_x = xv;
  lin_y = yv;
  plotColors = c;
  selectedChannels.resize(plotColors.size());
  selectedChannels.assign(plotColors.size(), true);

  if(!initData()){
    clearData();
    update();
    return(false);
  }
  update();
  return(true);
}

void ScatterPlotter::toggleLog(){
  plotLog = !plotLog;
  update();
}

void ScatterPlotter::setAlpha(int a){
  if(!plotColors.size())
    return;
  for(uint i=0; i < plotColors.size(); ++i)
    plotColors[i].setAlpha(a);
  update();
}

void ScatterPlotter::setSelection(vector<bool> b){
  if(b.size() == plotColors.size() && plotColors.size()){
    selectedChannels = b;
    update();
  }
}

bool ScatterPlotter::initData(){
  bool v_found = false;
  for(uint i=0; i < lin_x.size(); ++i){
    if(lin_x[i].size() && lin_y[i].size()){
      x_min = x_max = lin_x[i][0];
      y_min = y_max = lin_y[i][0];
      v_found = true;
      break;
    }
  }
  if(!v_found){
    cerr << "ScatterPlotter::initData no initial value found" << endl;
    return(false);
  }
  
  for(uint i=0; i < lin_x.size(); ++i){
    if(lin_x[i].size() != lin_y[i].size()){
      cerr << "ScatterPlotter::initData x and y sub vectors of different size " 
	   << i << "\t" << lin_x[i].size() << "," << lin_y[i].size() << endl;
    }
    for(uint j=0; j < lin_x[i].size(); ++j){
      x_min = x_min < lin_x[i][j] ? x_min : lin_x[i][j];
      x_max = x_max > lin_x[i][j] ? x_max : lin_x[i][j];
      y_min = y_min < lin_y[i][j] ? y_min : lin_y[i][j];
      y_max = y_max > lin_y[i][j] ? y_max : lin_y[i][j];
    }
  }

  setLogValues();
  return(true);
}

void ScatterPlotter::clearData(){
  lin_x.clear();
  lin_y.clear();
  log_x.clear();
  log_y.clear();
  plotColors.clear();
}

void ScatterPlotter::setLogValues(){
  float min = x_min < y_min ? x_min : y_min;
  float max = x_max > y_max ? x_max : y_max;
  
  logModifier = 0;
  if(min < 0)
    logModifier = min + max / 100.0;  // arbitrary..

  log_x.resize(lin_x.size());
  log_y.resize(lin_x.size());
  for(uint i=0; i < lin_x.size(); ++i){
    log_x[i].resize(lin_x[i].size());
    log_y[i].resize(lin_x[i].size());
    for(uint j=0; j < lin_x[i].size(); ++j){
      log_x[i][j] = log( logModifier + lin_x[i][j]);
      log_y[i][j] = log( logModifier + lin_y[i][j]);
    }
  }
}

void ScatterPlotter::paintEvent(QPaintEvent* e){
  e = e;
  float w = (float)(width() - hMargin * 2);
  float h = (float)(height() - vMargin * 2);
  
  float xmin = plotLog ? log(x_min + logModifier) : x_min;
  float xmax = plotLog ? log(x_max + logModifier) : x_max;
  float ymin = plotLog ? log(y_min + logModifier) : y_min;
  float ymax = plotLog ? log(y_max + logModifier) : y_max;

  if(!plotLog){
    xmin = xmin > 0 ? 0 : xmin;
    ymin = ymin > 0 ? 0 : ymin;
  }

  float xrange = xmax - xmin;
  float yrange = ymax - ymin;

  QPainter p(this);
  p.translate(0, height());
  p.scale(1.0, -1.0);
  p.translate(hMargin, vMargin);
  
  p.drawLine(0, 0, w, 0);
  p.drawLine(0, 0, 0, h);
  for(int xp = 0; xp <= (int)w; xp += tick_spacing)
    p.drawLine(xp, -tick_length, xp, 0);
  
  for(int yp = 0; yp <= (int)h; yp += tick_spacing)
    p.drawLine(-tick_length, yp, 0, yp);

  if(!xrange || !yrange){
    cerr << "ScatterPlotter::paintEvent empty range plotting nothing" << endl;
    return;
  }
  
  vector<vector<float> >& xv = plotLog ? log_x : lin_x;
  vector<vector<float> >& yv = plotLog ? log_y : lin_y;
  p.setBrush(Qt::NoBrush);
  
  for(uint i=0; i < xv.size(); ++i){
    if(!selectedChannels[i])
      continue;
    p.setPen(QPen(plotColors[i], 0));
    for(uint j=0; j < xv[i].size(); ++j){
      int xp = int(xScale * w * (xv[i][j] - xmin) / xrange);
      int yp = int(yScale * h * (yv[i][j] - ymin) / yrange);
      //p.drawPoint(xp, yp);
      p.drawEllipse(xp-m_size/2, yp-m_size/2, m_size, m_size);
      //p.drawRect(xp-m_size/2, yp-m_size/2, m_size, m_size);
    }
  }
}

void ScatterPlotter::keyPressEvent(QKeyEvent* e){
  switch(e->key()){
  case Qt::Key_Up:
    yScale *= 1.5;
    break;
  case Qt::Key_Down:
    yScale /= 1.5;
    break;
  case Qt::Key_Left:
    xScale /= 1.5;
    break;
  case Qt::Key_Right:
    xScale *= 1.5;
    break;
  case Qt::Key_Space:
    xScale = yScale = 1.0;
    break;
  default:
    e->ignore();
  }
  update();
}

void ScatterPlotter::mouseDoubleClickEvent(QMouseEvent* e){
  e = e;
  toggleLog();
}
