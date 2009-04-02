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

#ifndef TDPOINTDRAWER_H
#define TDPOINTDRAWER_H

#include "distanceMapper.h"
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
//#include <GL/glu.h>
#include <vector>

using namespace std;

class TDPointDrawer : public QGLWidget
{
  Q_OBJECT

    public :
    TDPointDrawer(QWidget* parent=0, const char* name=0);
  ~TDPointDrawer();

  public slots :
    void setModel(vector<dpoint> points);

  protected :
    void initializeGL();
  void paintGL();
  void resizeGL(int w, int h);

  void mousePressEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  //void paintEvent(QPaintEvent* e);   // necessary ?? -- don't think this is necessary.. 

  private :
    GLuint model;
  //GLUquadricObj* sphere;
  GLfloat xRot, yRot, zRot, xScale, yScale, zScale, xOffset, yOffset, zOffset; // useful things to have.. 
  int lastX, lastY;
  vector<dpoint> p;
  vector<float> maxValues;  
  vector<float> minValues;  // 3d space after all..
  float maxStress;
  float minStress;

};

#endif

    
