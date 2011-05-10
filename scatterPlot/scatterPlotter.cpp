#include "scatterPlotter.h"
#include <QPainter>
#include <QTextStream>
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
  origin = QPointF(hMargin, height() - vMargin);
  reportCurrentPoint = selectingPath = false;
  setFocusPolicy(Qt::StrongFocus);
  setDefaultColors();
}

bool ScatterPlotter::setData(vector<float> xv, vector<float> yv)
{
  vector<vector<float> > xvv;
  vector<vector<float> > yvv;
  xvv.push_back(xv);
  yvv.push_back(yv);
  return(setData(xvv, yvv));
}

bool ScatterPlotter::setData(vector<vector<float> > xv, vector<vector<float> > yv)
{
  return(setData(xv, yv, defaultColors));
}

bool ScatterPlotter::setData(vector<vector<float> > xv, vector<vector<float> > yv, vector<QColor> c)
{
  if(xv.size() != yv.size()){
    cerr << "ScatterPlotter::setData null or differently sized data vectors "
	 << xv.size() << "\t" << yv.size() << " : " << c.size() << endl;
    return(false);
  }
  origin = QPointF(hMargin, height() - vMargin);
  lin_x = xv;
  lin_y = yv;
  plotColors = c;
  selectedChannels.resize(plotColors.size());
  selectedChannels.assign(plotColors.size(), true);

  selectPath = QPainterPath();
  selectPath.setFillRule(Qt::WindingFill);
  selectingPath = false;

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

vector<vector<bool> > ScatterPlotter::selectPoints(bool filter){
    float w = (float)(width() - hMargin * 2);
    float h = (float)(height() - vMargin * 2);
    vector<vector<bool> > selection(lin_x.size());
    for(uint i=0; i < lin_x.size(); ++i){
	selection[i].resize(lin_x[i].size());
	if(!selectedChannels[i]){
	    selection[i].assign(selection[i].size(), filter);
	    continue;
	}
	// this boolean statement is a bit too clever by half. Be careful with it.
	for(uint j=0; j < lin_x[i].size(); ++j)
	    selection[i][j] = ( filter != 
				selectPath.contains( valueToPlotCoordinates(lin_x[i][j], lin_y[i][j], w, h )) );
    }
    return(selection);
}

void ScatterPlotter::setDefaultColors(){
  plotColors.push_back(QColor(200, 0, 0));
  plotColors.push_back(QColor(0, 200, 0));
  plotColors.push_back(QColor(0, 0, 200));
  plotColors.push_back(QColor(150, 0, 150));
  plotColors.push_back(QColor(0, 150, 150));
  plotColors.push_back(QColor(200, 125, 0));
  plotColors.push_back(QColor(120, 120, 200));
  defaultColors = plotColors;
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
  p.save();
  p.translate(0, height());
  p.scale(1.0, -1.0);
  p.translate(hMargin, vMargin);
  
  p.drawLine(0, 0, (int)w, 0);
  p.drawLine(0, 0, 0, (int)h);
  for(int xp = 0; xp <= (int)w; xp += tick_spacing)
    p.drawLine(xp, -tick_length, xp, 0);
  
  for(int yp = 0; yp <= (int)h; yp += tick_spacing)
    p.drawLine(-tick_length, yp, 0, yp);

  if(!xrange || !yrange){
    cerr << "ScatterPlotter::paintEvent empty range plotting nothing" << endl;
    return;
  }

  // if we have any selected path then draw that.
  p.setBrush(Qt::NoBrush);
  p.save();
  p.scale(xScale, yScale);
  p.drawPath(selectPath);
  p.restore();

  vector<vector<float> >& xv = plotLog ? log_x : lin_x;
  vector<vector<float> >& yv = plotLog ? log_y : lin_y;
  
  for(uint i=0; i < xv.size(); ++i){
    if(!selectedChannels[i])
      continue;
    p.setPen(QPen(plotColors[i % plotColors.size()], 0));
    for(uint j=0; j < xv[i].size(); ++j){
	float xf = xv[i][j];
	float yf = yv[i][j];
	valueToPlotCoordinatesR(xf, yf, w, h);
	int xp = (int)xf;
	int yp = (int)yf;
//      int xp = int(xScale * w * (xv[i][j] - xmin) / xrange);
//      int yp = int(yScale * h * (yv[i][j] - ymin) / yrange);
      //p.drawPoint(xp, yp);
      p.drawEllipse(xp-m_size/2, yp-m_size/2, m_size, m_size);
      //p.drawRect(xp-m_size/2, yp-m_size/2, m_size, m_size);
    }
  }

  // For any thing involving labels and such we need to restore the painter to it's 
  // original state..
  p.restore();

  if(reportCurrentPoint && 
     currentPoint.x() > hMargin && currentPoint.y() < (height() - vMargin)){
    QPointF pv = transformPos(QPointF(currentPoint));
    QPointF op = transformPos(origin); 
    float xv = xmin + (pv.x() / w) * xrange ;
    float yv = ymin + (pv.y() / h) * yrange ; 
    float xo = xmin + (op.x() / w) * xrange ;
    float yo = ymin + (op.y() / h) * yrange ; 
    float y_rate = (yv - yo) / (xv - xo);
    float k = yv - (y_rate * xv);

    p.setPen(QPen(QColor(50, 50, 50), 0));
    p.drawLine(hMargin, currentPoint.y(), currentPoint.x() + hMargin, currentPoint.y());
    p.drawLine(currentPoint.x(), currentPoint.y() - vMargin, currentPoint.x(), height() - vMargin);
    p.setPen(QPen(QColor(50, 50, 50), 0, Qt::DashLine));
    p.drawLine(currentPoint, origin);
    //    p.drawLine(currentPoint.x(), currentPoint.y(), hMargin, height() - vMargin);
    int lmargin = 3;
    QString numLabel;
    numLabel.setNum(xv);
    p.drawText(currentPoint.x() + lmargin + 10, currentPoint.y() + lmargin, width(), height(), 
	       Qt::AlignLeft|Qt::AlignTop, numLabel);
    numLabel.setNum(yv);
    p.drawText(0, 0, currentPoint.x() - lmargin, currentPoint.y() - lmargin,
	       Qt::AlignRight|Qt::AlignBottom, numLabel);
    numLabel = "";
    QTextStream qts(&numLabel);
    qts << "y=" << k << " + " << y_rate << "x";
    p.drawText(currentPoint.x() + hMargin, 0, width(), currentPoint.y() - vMargin,
	       Qt::AlignLeft|Qt::AlignBottom, numLabel);
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

void ScatterPlotter::mousePressEvent(QMouseEvent* e){
    if(e->button() == Qt::LeftButton){
	selectPath.moveTo(transformPos(e->posF()));
	selectingPath = true;
    }
    if(e->button() == Qt::RightButton){
      // report the position (draw three x, y, and diagonal
      reportCurrentPoint = true;
      currentPoint = e->pos();
      if(e->modifiers() & Qt::ControlModifier){
	origin =  QPointF(e->pos());
      }
      if(e->modifiers() & Qt::ShiftModifier){
	reportCurrentPoint = false;
      }
      update();
    }
}

void ScatterPlotter::mouseMoveEvent(QMouseEvent* e){
  currentPoint = e->pos();
  if(selectingPath){
    selectPath.lineTo(transformPos(e->posF()));
    update();
    return;
  }
  if(reportCurrentPoint)
    update();
}

void ScatterPlotter::mouseReleaseEvent(QMouseEvent* e){
    e = e;
    if(selectingPath){
      selectPath.closeSubpath();
      update();
    }
    selectingPath = false;
    //    origin = QPointF(hMargin, height() - vMargin);
}

void ScatterPlotter::mouseDoubleClickEvent(QMouseEvent* e){
  e = e;
  toggleLog();
}

QPointF ScatterPlotter::transformPos(QPointF p){
    qreal x = p.x();
    qreal y = p.y();
    x = (x - (float)hMargin) / xScale;
    y = (y - (height() - vMargin)) / -yScale;
    return(QPointF(x, y));
}

QPointF ScatterPlotter::valueToPlotCoordinates(float x, float y, float w, float h){
    float tx = x;
    float ty = y;
    valueToPlotCoordinatesR(tx, ty, w, h);
    return(QPointF(tx, ty));
}

void ScatterPlotter::valueToPlotCoordinatesR(float& x, float& y, float w, float h){
    float xmin = plotLog ? log(x_min + logModifier) : x_min;
    float xmax = plotLog ? log(x_max + logModifier) : x_max;
    float ymin = plotLog ? log(y_min + logModifier) : y_min;
    float ymax = plotLog ? log(y_max + logModifier) : y_max;
    if(!plotLog){
      xmin = xmin > 0 ? 0 : xmin;
      ymin = ymin > 0 ? 0 : ymin;
    }
    
    x = xScale * w * (x - xmin) / (xmax - xmin);
    y = yScale * h * (y - ymin) / (ymax - ymin);
}


// QPointF ScatterPlotter::logTransformPos(QPointF p){
//     QPointF linP = transformPos(p);
//     return( QPointF( log(linP.x()), log(linP.y()) ));
// }
