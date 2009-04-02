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

#include "pointViewWidget.h"
#ifdef Q_WS_WIN
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include <qgl.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <math.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;



void getMinMax(vector<float>* v, float& min, float& max){
  if(v->size() == 0){ return; }
  min = max = (*v)[0];
  for(int i=1; i < v->size(); i++){
    if(min > (*v)[i]) { min = (*v)[i]; }
    if(max < (*v)[i]) { max = (*v)[i]; }
  }
}



PointViewWidget::PointViewWidget(QWidget* parent, const char* name)
  : QGLWidget(parent, name)
{
  xRot = yRot = zRot = 0.0;
  xScale = yScale = zScale = 2.0;   // no good reason.. 
  scaleMod = 1.0;
  xOffset = yOffset = 0; 
  zOffset = -2;          
}

PointViewWidget::~PointViewWidget(){
  glDeleteLists( model, 1);
}

void PointViewWidget::initializeGL(){
  glClearColor(0.0, 0.0, 0.0, 0.0);
//  glEnable(GL_DEPTH_TEST);
//  glEnable( GL_ALPHA_TEST );
  glEnable( GL_BLEND );
  
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel( GL_FLAT );
}

void PointViewWidget::resizeGL(int w, int h){
  glViewport(0, 0, (GLint)w, (GLint)h );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
//  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);
//  glFrustum(-0.5, 0.5, -0.5, 0.5, 0.1, 100.0);
  glFrustum(-0.5, 0.5, -0.5, 0.5, 1.0, 10.0);
  glMatrixMode( GL_MODELVIEW );
  // I can probably remove some of these things
  // if I want to actively move the camera..
}

void PointViewWidget::paintEvent(QPaintEvent* e){
  makeCurrent();
  // I should really move this to a resize event, rather than letting it sit here
  // but I'm a lazy bastard... 
  resizeGL(width(), height());
  updateGL();
}

void PointViewWidget::mousePressEvent(QMouseEvent* e){
  lastX = e->x();
  lastY = e->y();
}

void PointViewWidget::mouseMoveEvent(QMouseEvent* e){
  switch(e->state()){
  case 1:
    // do something
    yRot += (GLfloat)(e->x() - lastX);
    xRot -= (GLfloat)(lastY - e->y());
    break;
  case 513:
    // left button plus control (maybe)
    zOffset += ( (GLfloat)(e->y() - lastY) )/( (GLfloat)(height()/20));
    break;
  case 4:
    // middle button.. 
    zOffset += ( (GLfloat)(e->y() - lastY) )/( (GLfloat)(height()/20));
    break;
  case 2:
    // the right button
    yOffset -= ( (GLfloat)(e->y() - lastY) )/( (GLfloat)(height()/5));
    xOffset += ( (GLfloat)(e->x() - lastX) )/( (GLfloat)(width()/5));
    break;
  default:
    // do nothing
    break;
  }
  // and update
  lastX = e->x();
  lastY = e->y();
  updateGL();
}

void PointViewWidget::wheelEvent(QWheelEvent* e){
    // change xScale, yScale and zScale.. 
    scaleMod = scaleMod * (1.0 + 0.1 * (GLfloat)(e->delta()/120)); 
    cout << "change scales by : " << scaleMod << endl;
//     xScale = xScale * scaleM;
//     yScale = yScale * scaleM;
//     zScale = zScale * scaleM;
	//xScale + (GLfloat)1.1 * 
	//cout << "changing scale from " << xScale << " to " << nScale << endl;
	//xScale = yScale = zScale = nScale;
    updateGL();
}

void PointViewWidget::paintGL(){
  glClear( GL_COLOR_BUFFER_BIT );
  glClear( GL_DEPTH_BUFFER_BIT );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glTranslatef(xOffset, yOffset, zOffset);
  glRotatef( xRot, 1.0, 0.0, 0.0 );
  glRotatef( yRot, 0.0, 1.0, 0.0 );
  glRotatef( zRot, 0.0, 0.0, 1.0 );

  glScalef(scaleMod * xScale, scaleMod * yScale, scaleMod * zScale );  // I probably don't need to control this

  //  cout << "calling model, xRot: " << xRot << "\tyRot " << yRot << endl;
  glCallList( model );
  glFlush();
}

void PointViewWidget::setModel(voxelVolume& volume){
  //cout << "setModel .. " << endl;
  makeCurrent();
  //cout << "model list number is " << model << endl;
  glDeleteLists( model, 1);
  model = glGenLists( 1 );
  //cout << "and now it is : " << model << endl;
  glNewList( model, GL_COMPILE );
  //cout << "and called glNewList " << endl;
  
  glLineWidth( 2.0 );
  glPointSize( 3.0 );    // draw points, !!

//  xScale = (GLfloat) pd.xLog ? (lmaxX ? 4/lmaxX : 1) : 4/pd.maxX;
//  yScale = (GLfloat) pd.yLog ? (lmaxY ? 4/lmaxY : 1) : 4/pd.maxY;
//  zScale = (GLfloat) pd.zLog ? (lmaxZ ? 4/lmaxZ : 1) : 4/pd.maxZ;
  //cout << "scales set " << endl;
  
  GLfloat xr = GLfloat(volume.w) / GLfloat(2.0);
  GLfloat yr = GLfloat(volume.h) / GLfloat(2.0);
  GLfloat zr = GLfloat(volume.d) / GLfloat(2.0);    // use for offsets.. (i.e. x - xr).. 

  // work out the individual scales using the relationships between w and width, h and height
  // and so on. Scale such that the largest distance becomes 1 (that's a bit ugly, but.. )

  // to work out the scaling factors we need to know the max dimension length;
  GLfloat maxDim = volume.width > volume.height ? volume.width : volume.height;
  maxDim = maxDim > volume.depth ? maxDim : volume.depth;

  // then each scale dimension can be given as ..
  xScale = volume.width / (maxDim * GLfloat(volume.w));
  yScale = volume.height / (maxDim * GLfloat(volume.h));
  zScale = volume.depth / (maxDim * GLfloat(volume.d));

  // if we scale by these parameters (and use a cr of 0.5) then everything should end up in positions between
  // -0.5 --> +0.5;   (ofcourse we have to consider translocating model in the z-scale) and the order in
  // which everything is done.

  GLfloat cr = 0.5;   // cube radius is 0.5 (width = 1.0),, 
  vector<drawVoxel>::iterator it;
  for(it = volume.voxels.begin(); it != volume.voxels.end(); it++){
      // work out where we are and set the appropriate colour
      // then call the drawCube with the appropriate numbers..
      GLfloat x = GLfloat((*it).x) - xr;
      GLfloat y = GLfloat((*it).y) - yr;
      GLfloat z = GLfloat((*it).z) - zr;
      glColor4f((*it).r, (*it).g, (*it).b, (*it).a);
      drawCube(x, y, z, cr);
  }
//   glColor4f(0.0, 1.0, 0.0, 0.25);
//   drawCube(-cr, cr/2.0, 0.0, 5.0);
//   glColor4f(0.0, 1.0, 0.0, 0.25);
//   drawCube(-cr, cr/2.0, 7.0, 2.5);
//   glColor4f(1.0, 1.0, 0.0, 0.25);
//   drawCube(-cr, cr/2.0, -5.0, 2.5);
//  glEnd();
  //cout << "made the points.. " << endl;
  glEndList();
  updateGL();
}
    
void PointViewWidget::drawCube(GLfloat  x, GLfloat y, GLfloat z, GLfloat w){
  makeCurrent();  // just to make sure..
  // make six polygons.. maybe there's a better way, but.. ?? 

   glBegin( GL_QUAD_STRIP );
//   glBegin( GL_LINES );
   glVertex3f(x+w, y-w, z-w);
   glVertex3f(x+w, y+w, z-w);
//   glColor4f(0.0, 1.0, 0.0, 0.25);
   glVertex3f(x-w, y-w, z-w);
   glVertex3f(x-w, y+w, z-w);
//   glColor4f(1.0, 0.0, 0.0, 0.25);
   glVertex3f(x-w, y-w, z+w);
   glVertex3f(x-w, y+w, z+w);
   glVertex3f(x+w, y-w, z+w);
//   glColor4f(0.0, 0.5, 0.5, 0.25);
   glVertex3f(x+w, y+w, z+w);
   glVertex3f(x+w, y-w, z-w);
//   glColor4f(0.0, 0.5, 1.0, 0.25);
   glVertex3f(x+w, y+w, z-w);
   glEnd();

//   glColor4f(0.75, 0.0, 0.75, 0.25);
   glBegin( GL_QUADS );
//   glBegin( GL_LINE_STRIP );
   glVertex3f(x-w, y+w, z-w);
   glVertex3f(x+w, y+w, z-w);
   glVertex3f(x+w, y+w, z+w);
   glVertex3f(x-w, y+w, z+w);
   glEnd();

   // glColor4f(0.0, 0.0, 0.75, 1.0);
   glBegin( GL_QUADS );
//   glBegin( GL_LINE_STRIP );
   glVertex3f(x-w, y-w, z-w);
   glVertex3f(x+w, y-w, z-w);
   glVertex3f(x+w, y-w, z+w);
   glVertex3f(x-w, y-w, z+w);
   glEnd();

//   glBegin( GL_TRIANGLE_FAN );
//   glVertex3f(x, y+w, z);
//   glVertex3f(x+w, y-w, z-w);
//   glVertex3f(x-w, y-w, z-w);
//   glVertex3f(x, y-w, z+w);
//   // draw the base of the pyramid.. 
//   glEnd();
//   glBegin( GL_TRIANGLES );
//   glVertex3f(x+w, y-w, z-w);
//   glVertex3f(x-w, y-w, z-w);
//   glVertex3f(x, y-w, z+w);
//   glEnd();
   
}

void PointViewWidget::generateNullModel(int r){
    if(r <= 2){    // 
	cerr << "be more ambitious,, 5x5x5 is too small ... to look good man.. " << endl;
	return;
    }
    // just generate a model .. using some thing..
    int w = r * 2 + 1;
    float sf = float(w) / float(5.0);  // some sort of scaling factor.. 
    voxelVolume vol(w, w, w, 1.0, 1.0, 1.0);
    for(int dz = -r; dz <= r; dz++){
	for(int dy = -r; dy <= r; dy++){
	    for(int dx = -r; dx <= r; dx++){
		// calculate some colour and some transparency and make a drawVoxel..
		float d = sqrt(GLfloat(dz * dz + dy * dy + dx * dx));  // i.e. the distance from the center..
		float i = expf(-d/sf);
		float a = i;
		//a = a < 0.5 ? 0.0 : a;
		a = a * a;
		//if(a > 0.25){
		vol.voxels.push_back(drawVoxel(dx+r, dy+r, dz+r, 1.0, 0.0, i, a));
		//}
	    }
	}
    }
    // and then we set the model with the volume..
    setModel(vol);
}
