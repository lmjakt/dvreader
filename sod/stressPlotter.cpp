//Copyright Notice
/*
    eXintegrator integrated expression analysis system
    Copyright (C) 2004  Martin Jakt & Okada Mitsuhiro
  
    This file is part of the eXintegrator integrated expression analysis system. 
    eXintegrator is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version. 

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#include "stressPlotter.h"
#include <qpainter.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <math.h>
#ifdef Q_OS_MACX
#include <limits.h>
#include <float.h>
#define MINFLOAT (FLT_MIN)
#else
#include <values.h>
#endif
#include <iostream>

using namespace std;

StressPlotter::StressPlotter(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    // don't do very much..
  setBackgroundMode(Qt::NoBackground);
  setCaption("Stress Plotter");
  maxValue = 0;
  minValue = 0;
  
  dimColors.push_back(new QColor(100, 0, 30));
  dimColors.push_back(new QColor(30, 0, 100));
  dimColors.push_back(new QColor(70, 0, 60));
  dimColors.push_back(new QColor(43, 43, 43));
  dimColors.push_back(new QColor(0, 70, 60));
  dimColors.push_back(new QColor(0, 100, 30));
  
  resize(300, 300);
}

StressPlotter::~StressPlotter(){
  for(uint i=0; i < dimColors.size(); ++i)
    delete dimColors[i];
}

void StressPlotter::setData(vector<stressInfo> stress){
  values = stress;
  //    maxValue = MINFLOAT;
  if(values.size())
    minValue = values[0].stress;
  for(uint i= values.size()/10 ; i < values.size(); ++i){
    maxValue = maxValue > values[i].stress ? maxValue : values[i].stress;
    minValue = minValue < values[i].stress ? minValue : values[i].stress;
  }
  maxValue = maxValue > 0 ? maxValue : 1.0;
  update();
}

void StressPlotter::paintEvent(QPaintEvent* e){
  int w = width();
  int h = height();
  float r = maxValue - minValue;
  r = (r == 0) ? maxValue : r;
  // draw some kind of background..
  QPixmap pix(w, h);
  pix.fill(QColor(0, 0, 0));
  QPainter p(&pix);
  drawStress(&p, w, h);
  
  bitBlt(this, 0, 0, &pix, 0, 0);
}

void StressPlotter::drawStress(QPainter* p, int w, int h){
  float r = maxValue - minValue;
  r = (r == 0) ? maxValue : r;
  // draw some kind of background..
  p->setPen(QPen(QColor(255, 255, 255), 1));
  p->setBrush(Qt::NoBrush);
  for(uint i= 0; i < values.size(); ++i){
    if(!values[i].stress)
      continue;
    int x = (w * i) / values.size();
    int y =  h - (int)(float(h) *  values[i].stress / maxValue);
    drawDims(p, x, values[i], h);
    //	int dimY = (int)(h - h * values[i].currentDF());
    //	p->setPen(QPen(QColor(100, 0, 30), 1));
    //	p->drawLine(x, height(), x, dimY);
    p->setPen(QPen(QColor(255, 255, 255), 1));
    p->drawEllipse(x, y, 4, 4);
  }
  if(values.size()){
    QString value;
    value.setNum(values.back().stress);
    p->drawText(0, 0, w, h, Qt::AlignRight|Qt::AlignBottom, value);
  }
}

void StressPlotter::drawDims(QPainter* p, int xp, stressInfo& si, int h){
  float dimMult = 1.0 / (float)si.dimFactors.size();
  float y1 = 1.0;
  for(uint i=0; i < si.dimFactors.size(); ++i){
    float y2 = y1 - (dimMult * si.dimFactors[i]);
    p->setPen(QPen(*(dimColors[ i % dimColors.size() ]), 1));
    p->drawLine(xp, (int)(h * y1), xp, (int)(h * y2));
    y1 = y2;
  }
}

