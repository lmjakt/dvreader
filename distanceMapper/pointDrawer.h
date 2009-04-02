//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
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

#ifndef POINTDRAWER_H
#define POINTDRAWER_H

#include "distanceMapper.h"
#include <qwidget.h>
#include <qrect.h>
#include <qpoint.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>
#include <vector>
#include <set>

using namespace std;

class PointDrawer : public QWidget
{
  Q_OBJECT

    public :
    PointDrawer(QWidget* parent=0, const char* name=0);     // it just draws things.. so no need.. 
  ~PointDrawer();

  void setData(vector<dpoint*> p);        // set the data, then redraw..
  void emptyData();                       // empty the points.. 
  
  private slots :
    void compareCellTypes();
  void setcoords();

  private :
    void paintEvent(QPaintEvent* e);
  void mousePressEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void checkSelected();

  // and somewhere to keep data for stuff in..
  vector<QPoint> selectPoints;
  vector<dpoint*> points;
  vector<QRect> regions;  // one region for each point.. so that we can check for mouse Events... 
  set<uint> selectedA;
  set<uint> selectedB;     // bit ugly but there you go.. 
  bool itsA;              // stupid if I ever saw something but there you go.. 
  QRect movingRect;    // 0 if nothing moving, otherwise, move it by 
  int movingId;
  float maxX, minX, maxY, minY, maxStress;    // need this for the painting..
  int diameter;                               // how big do we make the circles.. 
  int margin;                                 // how much of a margin do we want.
  int lastX, lastY;      // so that I can use QRegion::translate .. 
  
  Q3PopupMenu* menu;

  signals :
    void updatePosition(int, float, float);  // id, x, y, .... 
  void compareCells(vector<int>, vector<int>);
  void setCoordinates();             // ask my ower to do something about the coordinates .. pass them up the food chain please..
};

#endif
