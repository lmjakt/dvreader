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

#ifndef POINTVIEWWIDGET_H
#define POINTVIEWWIDGET_H

#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <vector>
#include <string>
#include "../dataStructs.h"


using namespace std;


class PointViewWidget : public QGLWidget
{
  Q_OBJECT

    public :
    PointViewWidget(QWidget* parent=0, const char* name=0);
  ~PointViewWidget();

  public slots:
      void setModel(voxelVolume& volume);   // contains stuff that can be thingimiyiggged..
  void generateNullModel(int w);            // generate some default model.. (something pretty maybe.. )
  
 protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int w, int h);
  void paintEvent(QPaintEvent* e);
  void mouseMoveEvent(QMouseEvent* e);  // for rotation and so on..
  void mousePressEvent(QMouseEvent* e); // for setting lastX and lastY
  void wheelEvent(QWheelEvent* e); // set the scale.. 

 private:
  GLuint model;   // created from a plotData struct in the setModel thingy. 
  GLfloat xRot, yRot, zRot, xScale, yScale, zScale, xOffset, yOffset, zOffset;
  GLfloat scaleMod;
  int lastX, lastY;
  void drawCube(GLfloat x, GLfloat y, GLfloat z, GLfloat w);  // useful that.. 
  

};

#endif
