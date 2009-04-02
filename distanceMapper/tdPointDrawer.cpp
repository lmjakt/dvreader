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

#include "tdPointDrawer.h"
#include "distanceMapper.h"
#include <vector>
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
//#include <GL/glu.h>
#include <iostream>

using namespace std;

TDPointDrawer::TDPointDrawer(QWidget* parent, const char* name)
  : QGLWidget(parent, name)
{
  xRot = yRot = zRot = 0.0;
  xScale = yScale = zScale = 1.0;   // though this will change ..
  xOffset = yOffset = 0;  // center everything..
  zOffset = -1;            // ??
  //sphere = 0;
  p.resize(0);
  maxValues.resize(3);
  minValues.resize(3);
  maxStress = minStress = 0;
}

TDPointDrawer::~TDPointDrawer(){
  cout << "deleting 3DPointDrawer " << endl;
}

void TDPointDrawer::initializeGL(){
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_FLAT);
}

void TDPointDrawer::resizeGL(int w, int h){
  glViewport(0, 0, (GLint)w, (GLint)h);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glFrustum(-1.0, 1.0, -1.0, 1.0, 0.5, 10.0);
  glMatrixMode( GL_MODELVIEW );
}

void TDPointDrawer::paintGL(){
  cout << "update gl" << endl;
  glClear( GL_COLOR_BUFFER_BIT );
  glClear( GL_DEPTH_BUFFER_BIT );
  
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glTranslatef(xOffset, yOffset, zOffset);
  glRotatef( xRot, 1.0, 0.0, 0.0 );
  glRotatef( yRot, 0.0, 1.0, 0.0 );
  glRotatef( zRot, 0.0, 0.0, 1.0 );

  glScalef(xScale, yScale, zScale );  // I probably don't need to control this
  cout << "scaled " << endl;
  //if(sphere){
    //}
  //sphere = gluNewQuadric();
  //cout << "made sphere " << endl;
  //gluQuadricDrawStyle(sphere, GLU_FILL);
  // and now go through and call all the spheres.. 
  //glPushMatrix();    // so I can translate all over the place..
  GLdouble radius = 1;
  if(p.size()){
    radius = (GLdouble)((maxValues[0] - minValues[0])/40.0);   // perhaps that is reasonable, who knows.. 
  }
  
  GLint slices = 15;
  GLint stacks = 15;
  for(int i=0; i < p.size(); i++){
    glPushMatrix();
    glTranslatef((GLfloat)p[i].coordinates[0], (GLfloat)p[i].coordinates[1], (GLfloat)p[i].coordinates[2]);
    GLfloat r = (GLfloat)  ( (p[i].stress-minStress)/(maxStress -minStress) );
    GLfloat g = 1-g;
    glColor3f(r, g, 0.0);
    //gluSphere(sphere, radius, slices, stacks);
    glPopMatrix();
  }
  //cout << "done the spheres.. " << endl;
  //gluDeleteQuadric(sphere);
  //sphere = 0;
  //cout << "deleted the sphere" << endl;
  //  cout << "calling model, xRot: " << xRot << "\tyRot " << yRot << endl;
  //glCallList( model );
  glFlush();
  //cout << "called flush " << endl;
}

void TDPointDrawer::setModel(vector<dpoint> points){
  p = points;
  cout << "setting model " << endl;
  if(p.size() < 1){
    return;
  }
  //  makeCurrent();
  //glDeleteLists( model, 1);
  //model = glGenLists(1);
  //glNewList( model, GL_COMPILE );

  //  glLineWidth( 1.0 );
  //glPointSize( 4.0 );   // though I'm not sure if we'll use the points or not..
  // a vector of maximum and minium coordinates
  // set the initial values for maxValues and minValues..
  //  cout << "setting maxes and mins" << endl;
  for(int i=0; i < 3; i++){
    maxValues[i] = minValues[i] = p[0].coordinates[i];   // CRASH if I don't have at least 3 coordinates
  }
  minStress = maxStress = p[0].stress;
  //cout << "set the mins and maxes " << endl;
  // main thing to do is to draw the force lines, may not need to draw anything else.. --circles take to much time (GLU command).. 
  //glBegin( GL_LINES );
  for(int i=0; i < p.size(); i++){
    // update the min and max values..
    for(int j=0; j < 3; j++){
      if(p[i].coordinates[j] > maxValues[j]) { maxValues[j] = p[i].coordinates[j]; }
      if(p[i].coordinates[j] < minValues[j]) { minValues[j] = p[i].coordinates[j]; }
    }
    if(maxStress < p[i].stress){ maxStress = p[i].stress; }
    if(minStress > p[i].stress){ minStress = p[i].stress; }
    // and just call the two points..
 //    for(int j=0; j < p[i].components.size(); j++){
//       // set the colour appropriately.. 
//       if(p[i].components[j].attractive){
// 	glColor3f(1.0, 1.0, 0.0);
//       }else{
// 	glColor3f(0.64, 0.596, 0.91);
//       }
//       glVertex3f((GLfloat)p[i].coordinates[0], (GLfloat)p[i].coordinates[1], (GLfloat)p[i].coordinates[2]);
//       // and the second point;
//       glVertex3f((GLfloat)(p[i].coordinates[0] + p[i].components[j].forces[0]), 
// 		 (GLfloat)(p[i].coordinates[1] + p[i].components[j].forces[1]), 
// 		 (GLfloat)(p[i].coordinates[2] + p[i].components[j].forces[2]) );
//     }
  }
  //glEnd();
  //// make spheres at the appropriate places.. hmm, using gluSphere

  //  glEndList();
  /// at this point we have to work out how the appropriate scaling and the appropriate offsets.. -probably not too tricky..
  // basically we want to constrain everythingy into a box which has 1.5 x 1.5 x 1.5 with 0, 0, 0 in the middle.. 
  // (our viewport opens to -1 to +1 but we want to have some margins.. )
  xScale = (GLfloat) ( 2.0 / (maxValues[0] - minValues[0]) );
  yScale = (GLfloat) ( 2.0 / (maxValues[1] - minValues[1]) );
  zScale = (GLfloat) ( 0.5 / (maxValues[2] - minValues[2]) );   // make it look kind of flat.. 
  // however, we now have to work out a reasonable offset.. -- and work out when it should be applied. 
  // model is translated first, so all I really have to do is make sure that the middle of the model is at the 0 position.. 
  //xOffset = (GLfloat)( -(maxValues[0] - minValues[0])/2 );
  //yOffset = (GLfloat)( -(maxValues[1] - minValues[1])/2 );
  //zOffset = (GLfloat)( 2.5 - (maxValues[2] - minValues[2])/2 );  // 

  updateGL();
}

void TDPointDrawer::mousePressEvent(QMouseEvent* e){
  lastX = e->x();
  lastY = e->y();
}

void TDPointDrawer::mouseMoveEvent(QMouseEvent* e){
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


