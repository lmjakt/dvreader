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

#include "pointDrawer.h"
#include "distanceMapper.h"    // for the dpoint .. bit messy but there you go..
#include <qwidget.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3PopupMenu>
#include <QPaintEvent>
#include <vector>
#include <qpainter.h>
#include <qcolor.h>
#include <qregion.h>
#include <qpixmap.h>
#include <iostream>
#include <q3pointarray.h>

using namespace std;

PointDrawer::PointDrawer(QWidget* parent, const char* name)
  : QWidget(parent, name)
{
    setBackgroundMode(Qt::NoBackground);    // but really should have something to put it there.. hmm
    movingId = -1;
  regions.resize(0);
  menu = new Q3PopupMenu(this, "menu");
  menu->insertItem("Compare Cell Types", this, SLOT(compareCellTypes()) );
  menu->insertItem("Set Coordinates", this, SLOT(setcoords()) );
}

PointDrawer::~PointDrawer(){
  cout << "destroying point drawer" << endl;
}

void PointDrawer::emptyData(){
  points.resize(0);
}

void PointDrawer::setData(vector<dpoint*> p){
  points = p;
  regions.resize(points.size());
  itsA = true;
  // and work out max, min, etc..
  if(points.size() < 1){
    maxX = minX = maxY = minY = 0;
  }
  if(points[0]->dimNo < 2){
    cerr << "we will be crashing soon. That really rather sucks a lot, but what can one do.. " << endl;
    return;
  }
  minX = maxX = points[0]->coordinates[0];
  minY = maxY = points[0]->coordinates[1];
  maxStress = points[0]->stress;
  for(int i=0; i < points.size(); i++){
    if(points[i]->coordinates[0] > maxX){ maxX = points[i]->coordinates[0]; }
    if(points[i]->coordinates[0] < minX){ minX = points[i]->coordinates[0]; }
    if(points[i]->coordinates[1] > maxY){ maxY = points[i]->coordinates[1]; }
    if(points[i]->coordinates[1] < minY){ minY = points[i]->coordinates[1]; }
    if(points[i]->stress > maxStress){ maxStress = points[i]->stress; }
  }
  // and call update.. !!! hooo hoo yeah....
  repaint();
//  update();
}

void PointDrawer::paintEvent(QPaintEvent* e){
  float forceMultiplier = 0.5;       // forces are too large, makes the picture too messy.. 
  // rather than try to work out areas and related stuff.. -just 
  // draw the whole thing to a pixmap and then bitBlt it.. shouldn't be 
  // too much of a problem.. 
  QPixmap pix(width(), height());
  pix.fill(QColor(0, 0, 0));     // black backround man it's good stuff.. 
  
  diameter = 16;             // diameter for circles representing.. things.. 
  margin = 25;               // number of pixels around the edge that we don't want to draw..
  

  if(points.size()){
    float xMult = ((float)(width() - 2 * margin))/(maxX - minX);
    float yMult = ((float)(height() - 2 * margin))/(maxY - minY);
    float stressMultiplier = 0;
    if(maxStress > 0){
      stressMultiplier = 254.0 / maxStress;     // not so good as we don't see any reduction in the max stress. 
    }
    // we might want to change this sometime later, so it is relative to start.
    
    QPainter p(&pix);              // should be ok.. 
    QString numString;  
    QPen attraction(QColor(255, 255, 0), 1);   // yellow
    QPen repulse(QColor(145, 152, 226), 1);    // light blue.. 
    // and let's go through the points and draw them as we see fit.. need some
    for(int i=0; i < points.size(); i++){
      if(points[i]->dimNo < 2){
	cerr << "??? bugger me backwards, but the coordinates size is less than two for i : " << i << endl;
	continue;
      }
      int x = (int)((points[i]->coordinates[0] - minX) * xMult) + margin;
      int y = (int)((points[i]->coordinates[1] - minY) * yMult) + margin;
      //  int r = (int)(points[i].stress * stressMultiplier);
      //  int g = 255 - r;
      // draw the component vectors as lines, either light blue or yellow.. 
      for(int j=0; j < points[i]->componentNo; j++){
	float fx = points[i]->coordinates[0] + forceMultiplier * points[i]->components[j]->forces[0];
	float fy = points[i]->coordinates[1] + forceMultiplier * points[i]->components[j]->forces[1];
	int x2 = (int)((fx - minX) * xMult) + margin;
	int y2 = (int)((fy - minY) * yMult) + margin;
	if(points[i]->components[j]->attractive){
	  p.setPen(attraction);
	}else{
	  p.setPen(repulse);
	}
	p.drawLine(x, y, x2, y2);   // x2 and y2 are forces and are relative to the current position.. 
      }
    }
    for(int i=0; i < points.size(); i++){
      if(points[i]->dimNo < 2){
	cerr << "??? bugger me backwards, but the coordinates size is less than two for i : " << i << endl;
	continue;
      }
      int x = (int)((points[i]->coordinates[0] - minX) * xMult) + margin - diameter/2;
      int y = (int)((points[i]->coordinates[1] - minY) * yMult) + margin - diameter/2;
      //int r = 255;   // temp solution
      //cout << "points[" << i << "]->stress is " << points[i]->stress << "  multiplier is : " << stressMultiplier << "max  Stress is : " << maxStress << endl;
      //cout << "Drawer : point " << i << "  index " << points[i]->index << "  stress : " << points[i]->stress << "  x: " << points[i]->coordinates[0] << "  y : " << points[i]->coordinates[1] << endl;

      int r = (int)(points[i]->stress * stressMultiplier);
      int g = 255 - r;
      int b = 0;
      cout << "\t\t\tr : " << r << "  g " << g << "\tmaxStress : " << maxStress << "\tmultiplier : " << stressMultiplier << "\tstress : " << points[i]->stress<< endl;
      
      if(selectedA.count(i)){
	r = 0;
	g = 75;
	b = 255;
      }
      if(selectedB.count(i)){
	r = 200;
	g = 0;
	b = 200;
      }
      p.setPen(Qt::NoPen);  // later maybe we'll take something from here.. 
      p.setBrush(QColor(r, g, b));
      p.drawEllipse(x, y, diameter, diameter);
      regions[i].setRect(x, y, diameter, diameter);
      
      // draw the number of the thing..
      p.setPen(QPen(QColor(255, 255, 255), 1));
      numString.setNum(points[i]->index);
      int extra = 10;
      p.drawText(x-extra, y, diameter+extra*2, diameter, Qt::AlignCenter, numString);
    }
    if(movingId != -1){
      p.setBrush(QColor(100, 100, 100));   // a gray shadow..
      p.setPen(Qt::NoPen);
      p.drawEllipse(movingRect);
    }
  }
  // ando bitblt.. 
  bitBlt(this, 0, 0, &pix, 0, 0);
  // and no need to delete anything as it's all .. nanni..
}

void PointDrawer::mousePressEvent(QMouseEvent* e){
  //cout << "mouse press event " << endl;
  // if the click is inside a thingy..
  if(e->button() == Qt::RightButton){
    menu->popup(mapToGlobal(e->pos()));
    return;
  }
  for(int i=0; i < regions.size(); i++){
    //cout << "  i: " << i << "  x : " << regions[i].x() << "  y: " << regions[i].y() << endl;
    if(regions[i].contains(e->pos())){
      movingId = i;
      movingRect = regions[i];
      //cout << "found a moving rectangle" << endl;
      break;
    }
  }
  selectPoints.resize(0);    //hmm may be better to do else where... but .. 
  if(movingId == -1){
    selectPoints.push_back(e->pos());
  }

  lastX = e->x();
  lastY = e->y();
}

void PointDrawer::mouseMoveEvent(QMouseEvent* e){
  //cout << "mouseMoveEvent" << endl;
  if(movingId != -1){
    //cout << "moving so moving the rect .. " << endl;
    movingRect.moveBy(e->x() - lastX, e->y() - lastY);
  }
  if(selectPoints.size()){
    QPainter p(this);
    p.setPen(QPen(QColor(255, 255, 255), 1));
    //p.drawPoint(e->pos());
    p.drawLine(lastX, lastY, e->x(), e->y());
    selectPoints.push_back(e->pos());
  }
  lastX = e->x();
  lastY = e->y();
  if(movingId != -1){
    update();
  }
}

void PointDrawer::mouseReleaseEvent(QMouseEvent* e){
  // and emit something useful here so the mapper changes the coordinates and tries to update things..
  // do in a couple of different steps.. 
  if(movingId != -1){
    float xMult = ((float)(width() - 2 * margin))/(maxX - minX);
    float yMult = ((float)(height() - 2 * margin))/(maxY - minY);
    float x = minX + ((float)(movingRect.x() + diameter/2 - margin))/xMult;
    float y = minY + ((float)(movingRect.y() + diameter/2 - margin))/yMult;
    emit updatePosition(movingId, x, y);    // catch in the viewer, pass on to the mapper, and continue the mapping.. 
  }
  if(selectPoints.size()){
    checkSelected();
  }
  movingId = -1;
  update();
}

void PointDrawer::checkSelected(){
  if(selectPoints.size() < 2){
    cerr << "Don't have any selected points.. ";
    return;
  }
  ///// Here's something really ugly,, create a QPointArray from the selectPoints.. -- why ? because QPointArray
  ///// doesn't have any push, or reserve, or other useful things... so waste of time.. OK..
  Q3PointArray pts(selectPoints.size());
  for(uint i=0; i < selectPoints.size(); i++){
    pts.setPoint(i, selectPoints[i]);        // ugly, if I ever saw something...
  }
  // make a QRegion with these points.. and then check to see what's inside ..
  QRegion r(pts);
  // and now we can go through and check all of the things and see which ones belong..
  float xMult = ((float)(width() - 2 * margin))/(maxX - minX);
  float yMult = ((float)(height() - 2 * margin))/(maxY - minY);
  set<uint>* a;
  set<uint>* b;
  cout << "itsA is " << itsA << endl;
  if(itsA){
    cout << "set A to be A " << endl;
    a = &selectedA;
    b = &selectedB;
  }else{
    cout << "set B to be a " << endl;
    b = &selectedA;
    a = &selectedB;
  }
  itsA = !itsA;
  cout << "what's up?" << endl;
  // empty a..
  while(a->size()){ a->erase(a->begin()); }
  for(int i=0; i < points.size(); i++){
    int x = (int)((points[i]->coordinates[0] - minX) * xMult) + margin;
    int y = (int)((points[i]->coordinates[1] - minY) * yMult) + margin;
    if(r.contains(QPoint(x, y))){
      cout << "Point with index : " << points[i]->index << " is contained by the region " << endl;
      // stick everything into a..
      b->erase(i);          // don't check, just call erase, no harm if nothing there.. 
      a->insert(i);         // use the index from the vector rather than the internal index, it's easier..
    }
  }
  cout << "and then what ? " << endl;
  // and return ..
}

void PointDrawer::compareCellTypes(){
  if(!selectedA.size() || !selectedB.size()){
    cerr << "Cant compare nothing against something, now can I, you fool !!" << endl;
    return;
  }
  cout << "Should be emitting something useful for the parents to take care of.. " << endl;
  vector<int> a;
  vector<int> b;
  set<uint>::iterator it;
  for(it = selectedA.begin(); it != selectedA.end(); it++){
    // ugly,,
    if(*it < points.size()){
      a.push_back(points[*it]->index);
    }
  }
  for(it = selectedB.begin(); it != selectedB.end(); it++){
    if(*it < points.size()){
      b.push_back(points[*it]->index);
    }
  }
  emit compareCells(a, b);     // the order doesn't matter so much.. 
		  
}

void PointDrawer::setcoords(){
  cout << "point drawer emitting setCoordinates : " << endl;
  emit setCoordinates();  // and let our owner -- have a look at what the coordinates really are.. hmm .
}
