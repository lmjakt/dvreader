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

#ifndef STRESSPLOTTER_H
#define STRESSPLOTTER_H

#include <QWidget>
#include <vector>
#include "distanceMapper.h"
#include <QColor>

class QPainter;

class StressPlotter : public QWidget
{
    Q_OBJECT
      public :
  StressPlotter(QWidget* parent=0, const char* name=0);
  ~StressPlotter();
  void setData(std::vector<stressInfo> stress);
  void setStressRange(float min, float max);
  void resetStressRange();
  void postscript(QString fname, float w, float h);

 private :
  void paintEvent(QPaintEvent* e);
  void drawStress(QPainter* p, int w, int h, bool black=false);
  void drawDims(QPainter* p, int xp, stressInfo& si, int h, int dim_box_width);
  std::vector<stressInfo> values;
  float maxValue;
  float minValue;
  
  std::vector<QColor*> dimColors;
  
};

#endif
