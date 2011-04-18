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

#ifdef Q_WS_WIN
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include <qgl.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include "glImage.h"
#include <qimage.h>
#include <iostream>
#include <string.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpen.h>
#include <math.h>

using namespace std;


/*!
  Create a GLImage widget
*/

GLImage::GLImage(unsigned int width, unsigned int height, unsigned int texSize, GLfloat aspRatio, QWidget* parent, const char* name )
  : QGLWidget( parent, name )
{
  viewingState = VIEW;
  setMouseTracking(true);
  xo = yo = 0;
  buttonPressed = Qt::NoButton;
  xscale = yscale = 1;
  aspectRatio = aspRatio;
  
  twidth = width;
  theight = height;
  textureWidth = texSize;
  textureHeight = texSize;
  textures = 0;   // reassign when initialising.. 
  overlay_textures = 0;

  xCross = yCross = 0.0;
  drawCross = false;
  showOverlay=false;
  // width and height are used to work out the backgroundHeight and backgroundWidth...
  // In fact it seems that backgroundWidth and imageWidth are never different from textureWidth
  // and I should remove them.
 
  setFocusPolicy(Qt::ClickFocus);
  setAttribute(Qt::WA_NoSystemBackground);
}

GLImage::GLImage(unsigned int width, unsigned int height, unsigned int texWidth, 
		 unsigned int texHeight, GLfloat aspRatio, QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
    setMouseTracking(true);
    xo = yo = 0;
    buttonPressed = Qt::NoButton;
    xscale = yscale = 1;
    aspectRatio = aspRatio;

    twidth = width;
    theight = height;
    textureWidth = texWidth;
    textureHeight = texHeight;
    textures = 0;   // reassign when initialising.. 
    overlay_textures = 0;

    xCross = yCross = 0.0;
    drawCross = false;
    showOverlay = false;

    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_NoSystemBackground);

}

/*!
  Release allocated resources
*/

GLImage::~GLImage()
{
    makeCurrent();
 
}


void GLImage::setImage(float* data, int x, int y, int col, int row){
    // first check that the data will fit and that we have an appropriate texture..
    if(col >= twidth || row >= theight){
	cerr << "GLImage::setImage row or height is too large row : " << row << "  col : " << col << endl;
	return;
    }
    if(x > textureWidth || y > textureHeight){
	cerr << "GLImage::setImage x or y seems to be too large  x :" << x << "  y : " << y  << endl;
	return;
    }
/////////////////////////// important stuff follows from here.. I think.. 

    int texPos = row * twidth + col;
    
    makeCurrent();
    glBindTexture(GL_TEXTURE_2D, textures[texPos]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGB, GL_FLOAT, data);

}

// maps a large image across the set of textures.
// source_x and source_y are the positions where we should position the data
// not the coordinates to to start from.. 
void GLImage::setBigImage(float* data, int source_x, int source_y,
		       		       int width, int height)
{
  if(width <= 0 || height <= 0)
    return;
  int panelCount = 0;
  for(int y=0; y < theight; ++y){
    for(int x=0; x < twidth; ++x){
      if(mapImageToTexture(data, source_x, source_y, width, height, 
			   x * textureWidth, y * textureHeight, textures[y * twidth + x])
	 )
	panelCount++;
    }
  }
}

void GLImage::setBigOverlay(unsigned char* data, int source_x, int source_y, int width, int height){
  if(width <= 0 || height <= 0)
    return;
  for(int y=0; y < theight; ++y){
    for(int x=0; x < twidth; ++x){
      mapOverlayToTexture(data, source_x, source_y, width, height,
			  x * textureWidth, y * textureHeight, overlay_textures[y * twidth + x]);
    }
  }
  showOverlay=true;
}

void GLImage::clearTextures(){
  float* blank = new float[ 3 * textureWidth * textureHeight ];
  memset((void*)blank, 0, sizeof(float) * 3 * textureWidth * textureHeight);
  for(int y=0; y < theight; ++y){
    for(int x=0; x < twidth; ++x){
      glBindTexture(GL_TEXTURE_2D, textures[y * twidth + x]);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGB, GL_FLOAT, blank);
    }
  }
  updateGL();
  delete blank;
}

void GLImage::gl_mod_buffer(float* destination, float* source, GLint w, GLint h, GLfloat param){
    makeCurrent();
    glDrawBuffer(GL_BACK);
    glRasterPos2i(0, 0);
    glDrawPixels(w, h, GL_RGB, GL_FLOAT, source);
//    glPixelTransferf(GL_RED_SCALE, param);
//    glFlush();
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGB, GL_FLOAT, destination); 
//    glPixelTransferf(GL_RED_SCALE, 1.0);
    // is it that simple ? 
}

bool GLImage::mapImageToTexture(float* image_data, int source_x, int source_y,
				int source_w, int source_h, int tex_x, int tex_y, GLuint texture)
{
  texture_overlap olap = sourceTextureOverlap(source_x, source_y, source_w, source_h, tex_x, tex_y);
  if(!olap.cp_w || !olap.cp_h)
    return(false);

  float* texture_data = new float[ 3 * olap.cp_h * olap.cp_w];
  // we are using an RGB triplet, so we need to multiply by three everywhere.. 
  if(olap.cp_w == textureWidth && olap.cp_w == source_w){
    memcpy((void*)(texture_data),
	   (void*)(image_data + (3 * olap.s_by * source_w)),
	   sizeof(float) * 3 * olap.cp_w * olap.cp_h);
  }else{
    for(int dy=0; dy < olap.cp_h; ++dy){
      memcpy((void*)(texture_data + (3 * dy * olap.cp_w )),
	     (void*)(image_data + (3 * ((dy + olap.s_by) * source_w + olap.s_bx) ) ),
	     sizeof(float) * 3 * olap.cp_w);
    }
  }
  // if we get here then 
  makeCurrent();
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, olap.t_bx, olap.t_by, olap.cp_w, olap.cp_h, GL_RGB, GL_FLOAT, texture_data);
  delete(texture_data);
  return(true);
}

// Assumes that texture data is in RGBA GL_UNSIGNED_BYTE format.. (4 components)
bool GLImage::mapOverlayToTexture(unsigned char* image_data, int source_x, int source_y,
				int source_w, int source_h, int tex_x, int tex_y, GLuint texture)
{
  texture_overlap olap = sourceTextureOverlap(source_x, source_y, source_w, source_h, tex_x, tex_y);
  if(!olap.cp_w || !olap.cp_h)
    return(false);

  unsigned char* texture_data = new unsigned char[ 4 * olap.cp_h * olap.cp_w];
  // we are using an RGB triplet, so we need to multiply by three everywhere.. 
  if(olap.cp_w == textureWidth && olap.cp_w == source_w){
    memcpy((void*)(texture_data),
	   (void*)(image_data + (4 * olap.s_by * source_w)),
	   sizeof(unsigned char) * 4 * olap.cp_w * olap.cp_h);
  }else{
    for(int dy=0; dy < olap.cp_h; ++dy){
      memcpy((void*)(texture_data + (4 * dy * olap.cp_w )),
	     (void*)(image_data + (4 * ((dy + olap.s_by) * source_w + olap.s_bx) ) ),
	     sizeof(unsigned char) * 4 * olap.cp_w);
    }
  }
  // if we get here then 
  makeCurrent();
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, olap.t_bx, olap.t_by, olap.cp_w, olap.cp_h, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
  delete(texture_data);
  return(true);
}


texture_overlap GLImage::sourceTextureOverlap(int source_x, int source_y, int source_w, int source_h,
					      int tex_x, int tex_y){
  texture_overlap ol;
  if( !((source_x < tex_x + textureWidth) && 
	(source_x + source_w > tex_x) && 
	(source_y < tex_y + textureHeight) && 
	(source_y + source_h) > tex_y)){
    return(ol);
  }
    // define the following variables
  // s_bx, t_bx : source and texture begin points
  // cp_w, cp_h : cp width and heights.
  // and then same in the y direction.

  int s_bx = tex_x > source_x ? tex_x - source_x : 0;
  int t_bx = tex_x >= source_x ? 0 : source_x - tex_x;
 
  int s_by = tex_y > source_y ? tex_y - source_y : 0;
  int t_by = tex_y >= source_y ? 0 : source_y - tex_y;
  
  // the maximum width and height that we can copy to and from 
  int max_w = textureWidth < source_w ? textureWidth : source_w;
  int max_h = textureHeight < source_h ? textureWidth : source_h;

  int cp_w = (tex_x + textureWidth) > (source_x + source_w) ? 
    ((source_x + source_w) - tex_x) : ((tex_x + textureWidth) - source_x);
  cp_w = cp_w > max_w ? max_w : cp_w;

  int cp_h = (tex_y + textureHeight) > (source_y + source_h) ?
    ((source_y + source_h) - tex_y) : ((tex_y + textureHeight) - source_y);
  cp_h = cp_h > max_h ? max_h : cp_h;

  if(cp_w <= 0 || cp_h <= 0){
    cerr << "GLImage::sourceTextureOverlap cp_w or cp_h less than 0: " << cp_w << "," << cp_h 
	 << "  source, etc: " << source_x << "," << source_y << " : " << source_w << "," << source_h
	 << "  texture: " << tex_x << "," << tex_y  << endl;
    return(ol);
  }
  if(t_bx < 0 || t_by < 0 || s_bx < 0 || s_by < 0){
    cerr << "GLImage::sourceTextureOverlap negative coordinates given : "
	 << t_bx << "," << t_by << "  : " << s_bx << "," << s_by << endl;
    return(ol);
  }
  ol.s_bx = s_bx;
  ol.s_by = s_by;
  ol.t_bx = t_bx;
  ol.t_by = t_by;
  ol.cp_w = cp_w;
  ol.cp_h = cp_h;
  return(ol);
}

void GLImage::setMagnification(float m){
  xscale = yscale = m;
  updateGL();
}

void GLImage::setPosition(int x, int y)
{
  // this assumes that the viewport is set to textureWidth and textureHeight
  // no real point to assuem this, but.. 
  xo = 1-(2.0 * (GLfloat)x) / (GLfloat)textureWidth;
  yo = 1-(2.0 * (GLfloat)y) / (GLfloat)textureHeight;
  // note that I might need to take into account the aspectRatio as well, but
  // not sure at the moment.. 
  updateGL();
}

void GLImage::setViewState(ViewState vstate)
{
  viewingState = vstate;
}

void GLImage::currentView(int& x, int& y, int& w, int& h)
{
  transformPos(0, height(), x, y, false);
  int x2, y2;
  transformPos(width(), 0, x2, y2);
  w = 1 + x2 - x;
  h = 1 + y2 - y;
}

void GLImage::transformPos(int x, int y, int& px, int& py, bool setCross){
    px = py = 0;   // in case something doesn't add up at least some kind of reasonable number.. 
    // work out our current coordinates using the offset and the scale values... not sure exactly how, but..
    
    // This is a really long way around, and it is ugly. I suppose that it can be summarised nicely into a single
    // nice equation, but I had a little trouble working out how to do this, so am trying to keep it simple.
    
    // I think this might end up being different if the size of the drawn pixmap is different from the background, 
    // and if the aspectRatio isn't one. (but you never know, it might work..)
    GLfloat frustumPos = -1.0 + ((GLfloat)(2 * x)/(GLfloat)textureWidth);
    GLfloat xOrigin = (-1.0 + xo) * xscale;
    GLfloat frustumDistance = frustumPos - xOrigin;
    GLfloat scaledFrustumDistance = frustumDistance / xscale;
    px = textureWidth * (scaledFrustumDistance / 2.0);
    
    // y position might be a little bit more complicated, but as long as the aspect ratio is 1, and the whole image is used... no problem.. 
    GLfloat yFrustumPos = -aspectRatio + ((GLfloat)(2 * (height() - y)) / (GLfloat)textureHeight);   //
    GLfloat yOrigin = (-aspectRatio + yo) * xscale;
    GLfloat yScaledFrustumDistance = (yFrustumPos - yOrigin) / xscale;
    py = textureHeight * (yScaledFrustumDistance / 2.0);
    
    // if we want to draw a cross on the thingy.. 
    if(setCross){ 
	xCross = scaledFrustumDistance - 1.0;
	yCross = yScaledFrustumDistance - aspectRatio;   
    }	

}

void GLImage::mouseDoubleClickEvent(QMouseEvent* e){
    drawCross = !drawCross;
    updateGL();
}

void GLImage::mousePressEvent(QMouseEvent* e){
    // if we have a control left click, then report our current position so that we can set slice positions.
    // These should be in terms of pixel positions, so the y position might be difficult to deal with.
    // at a later stage we might want to draw something to indicate this positon..
  int px, py;
  transformPos(e->x(), e->y(), px, py, true);
  buttonPressed = e->button();
  if(viewingState != VIEW){
    cout << "emitting mousePressed" << endl;
    emit mousePressed(QPoint(px, py), buttonPressed);
    return;
  }
  
  if(e->state() == (Qt::ControlButton)){
    emit newPos(px, py);
    buttonPressed = Qt::NoButton;  // if the mouse is moved do nothing
    if(drawCross){
      updateGL();
    }
    return;
  }
  if(e->button() == Qt::MidButton){
    // we need to set some variables here...
    lineStart = e->pos();
  }
  
  lastX = e->x();
  lastY = e->y();
  buttonPressed = e->button();
}

void GLImage::mouseMoveEvent(QMouseEvent* e){
  int px, py;
  transformPos(e->x(), e->y(), px, py, false);

  if(viewingState != VIEW){
    emit mouseMoved(QPoint(px, py), buttonPressed);
    return;
  }

  switch(buttonPressed){
  case Qt::LeftButton :
    xo += 2.0 * ((GLfloat)(e->x() - lastX)) / (xscale * (GLfloat)textureWidth) ;
    yo += aspectRatio * 2.0 * (GLfloat)(lastY - e->y()) / (xscale * (GLfloat)textureWidth * aspectRatio);   
    // vertical direction is opposite on these things
    // the offsets need to be divided by the multiplier.. hmm
    emit offSetsSet((int)(xo * (float)textureWidth), (int)(yo * (float)textureHeight));   
    break;
  case Qt::RightButton :
    // change the magnification multiplier..
    xscale += (lastY - e->y()) / (GLfloat)(200);
    emit magnificationSet(xscale);
    break;
  case Qt::MidButton :
    // well, we can try to think of something.. but
    lineEnd = e->pos();
    break;
  case Qt::NoButton :
    emit mousePos(px, py);
    break;
  default :
    cerr << "Unknown mouse button : " << e->button() << endl;
  }    
  lastY = e->y();
  lastX = e->x();
  
  updateGL();
  // and if we have something like this, make a QPainter and draw a line.. 
  if(buttonPressed == Qt::MidButton){
    QPainter p(this);
    p.setPen(QPen(QColor(255, 255, 255), 1));
    p.drawLine(lineStart, lineEnd);
  }
}

void GLImage::mouseReleaseEvent(QMouseEvent* e){
  int px, py;
  transformPos(e->x(), e->y(), px, py);

  if(viewingState != VIEW){
    emit mouseReleased(QPoint(px, py), buttonPressed);
    buttonPressed = Qt::NoButton; 
    return;
  }

  if(buttonPressed == Qt::MidButton){
    int x1, x2, y1, y2;
    transformPos(lineStart.x(), lineStart.y(), x1, y1, false);
    transformPos(lineEnd.x(), lineEnd.y(), x2, y2, false);
    emit newLine(x1, y1, x2, y2);   // and whatever..
  }
  buttonPressed = Qt::NoButton; 
}


void GLImage::wheelEvent(QWheelEvent* e){
  //emit incrementImage(e->delta() / 120);  // +/- 1  .. should be anyway
    if(e->delta() > 0){
	emit nextImage();
	return;
    }
    emit previousImage();
    // wheels emit -120 / + 120.. so basically.. divide by delta..
}

void GLImage::keyPressEvent(QKeyEvent* e){
  int key = e->key();

  switch(key){
  case Qt::Key_Right :
    emit nextImage();
    break;
  case Qt::Key_Up :
    emit nextImage();
    break;
  case Qt::Key_Left :
    emit previousImage();
    break;
  case Qt::Key_Down :
    emit previousImage();
    break;
  case Qt::Key_Home :
    emit firstImage();
    break;
  case Qt::Key_End :
    emit lastImage();
    break;
  case Qt::Key_V :
      viewingState = VIEW;
      break;
  default :
    e->ignore();
  }
  if(e->modifiers() == Qt::ControlModifier){
    switch(key){
    // case Qt::Key_V :
    //   viewingState = VIEW;
    //   break;
    case Qt::Key_D :
      viewingState = DRAW;
      break;
    default :
      e->ignore();
    }
  }

  emit keyPressed(e);
}

void GLImage::paintGL(){

    // See if we can use a normal painter here..
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glPopAttrib();
    glPushMatrix();
// use a texture to draw on..
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    GLfloat x = (GLfloat)textureWidth / (GLfloat)textureWidth;
    GLfloat y1 = -aspectRatio;
    GLfloat y2 = (textureHeight / textureWidth) * 2.0 * (aspectRatio * (GLfloat)textureHeight / (GLfloat)textureHeight) - aspectRatio;

    GLfloat h = y2 - y1;
    
    if(drawCross){
	glLineWidth(1.0);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(-1, (yCross + yo) * xscale, 0.0);
	glVertex3f(1, (yCross + yo) * xscale, 0.0);
	glVertex3f((xCross + xo) * xscale, -aspectRatio, 0.0);
	glVertex3f((xCross + xo) * xscale, aspectRatio, 0.0);
	glEnd();
    }

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for(int i=0; i < theight; i++){
	for(int j=0; j < twidth; j++){
	    GLint texture = textures[i * twidth + j];
	    
	    
	    // ok do the loop here to put all the stuff into appropriate places.. 
	    glBindTexture(GL_TEXTURE_2D, texture);
	    glBegin(GL_QUADS);
	    
	    GLfloat xoo = j * 2 * x;
	    GLfloat yoo = i * h;

	    glTexCoord2f(0, 0); glVertex3f((-x + xo + xoo) * xscale, (y1 + yo + yoo) * xscale, 0);
	    glTexCoord2f(0, 1.0); glVertex3f((-x + xo + xoo) * xscale, (y2 + yo + yoo) * xscale, 0);
	    glTexCoord2f(1.0, 1.0); glVertex3f((x + xo + xoo) * xscale, (y2 + yo + yoo) * xscale, 0);
	    glTexCoord2f(1.0, 0); glVertex3f((x + xo + xoo) * xscale, (y1 + yo + yoo) * xscale, 0);
	    
	    glEnd();

	    if(showOverlay){
	      glBindTexture(GL_TEXTURE_2D, overlay_textures[i * twidth + j]);
	      glBegin(GL_QUADS);
	      
	      GLfloat xoo = j * 2 * x;
	      GLfloat yoo = i * h;
	      
	      glTexCoord2f(0, 0); glVertex3f((-x + xo + xoo) * xscale, (y1 + yo + yoo) * xscale, 1);
	      glTexCoord2f(0, 1.0); glVertex3f((-x + xo + xoo) * xscale, (y2 + yo + yoo) * xscale, 1);
	      glTexCoord2f(1.0, 1.0); glVertex3f((x + xo + xoo) * xscale, (y2 + yo + yoo) * xscale, 1);
	      glTexCoord2f(1.0, 0); glVertex3f((x + xo + xoo) * xscale, (y1 + yo + yoo) * xscale, 1);
	      
	      glEnd();
	    }

	}
    }
    glFlush();
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();

}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLImage::initializeGL()
{
  


//////    // check how much space we have for gltextures..
//     GLint ww;
//     cout << endl;
//     glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 2048, 2048, 0, GL_RGB, GL_FLOAT, NULL);
//     glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww);
//     cout << "check for 2048 & 2048 ww is " << ww << endl;

//     glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 4096, 4096, 0, GL_RGB, GL_FLOAT, NULL);
//     glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww);
//     cout << "check for 4096 & 4096 ww is " << ww << endl;

//     glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 4096, 4096, 0, GL_RGB, GL_FLOAT, NULL);
//     glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww);
//     cout << "check for 4096 & 4096 ww is " << ww << endl;

//     glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 8192, 4096, 0, GL_RGB, GL_FLOAT, NULL);
//     glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww);
//     cout << "check for 8192 & 4096 ww is " << ww << endl;

//     cout << endl;
//////////////////////////////////

//////////////////////////// generate twidth & theight textures.. 

  if(textures){
    delete textures;
  }
  backgroundImage = make_background(textureWidth, textureHeight);
  
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // note if we use RGBA or we use floats we shouldn't need to call this.. (but for now the example.._)
  textures = new GLuint[theight * twidth];
  glGenTextures(twidth * theight, textures);    // makes the required number of textures.. 
  for(int i=0; i < theight; i++){
    for(int j=0; j < twidth; j++){
      glBindTexture(GL_TEXTURE_2D, textures[i * twidth + j]);
      //glGenTextures(1, &texName);
      //	    glBindTexture(GL_TEXTURE_2D, texName);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, backgroundImage);
      GLint resWidth;
      glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &resWidth);
      
    }
  }

  // I tried putting the below code into a separate function, bt for some reason, that results in lots of white squares;
  // I don't quite understand that (presumably some state changes when we leave this function body, but I don't quite understand
  // that.. So for now leave it here, and comment out the generate_overlay_textures function.. 
  //generate_overlay_textures();
  // ////////////////// and generate an additional texture for overlays.. 
  // // use GL_BYTE as the internal storage, then 4 * byte for RGBA
  unsigned char* test_overlay_image = new unsigned char[textureWidth * textureHeight * 4];
  memset((void*)test_overlay_image, 0, sizeof(unsigned char) * 4 * textureWidth * textureHeight);
  // and then make a circle or something .. 
  for(int r=50; r < 75; ++r){
    for(int dy=-r; dy <= r; ++dy){
      int dx = (int)sqrt( (float)( (r * r) - (dy * dy) ) );
      int y = (textureHeight / 2) + dy;
      int x1 = (textureWidth / 2) - dx;
      int x2 = (textureWidth / 2) + dx;
      int off1 = 4 * (y * textureWidth + x1);
      int off2 = 4 * (y * textureWidth + x2);
      
      test_overlay_image[ off1 ] = 255;
      test_overlay_image[ off1 + 1] = 255;
      test_overlay_image[ off1 + 2] = 255;
      test_overlay_image[ off1 + 3] = 125;
      
      test_overlay_image[ off2 ] = 125;
      test_overlay_image[ off2 + 1] = 125;
      test_overlay_image[ off2 + 2] = 125;
      test_overlay_image[ off2 + 3] = 50;
    }
  }
  
    
  if(overlay_textures)
    delete overlay_textures;
  overlay_textures = new GLuint[theight * twidth];
  glGenTextures(twidth * theight, overlay_textures);
  for(int i=0; i < theight; i++){
    for(int j=0; j < twidth; j++){
      glBindTexture(GL_TEXTURE_2D, overlay_textures[i * twidth + j]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, test_overlay_image);
    }
  }    
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLImage::resizeGL( int w, int h )
{
    glViewport(0, 0, w, h);  // but then we have to change the width of the frustum or the orthogonal to ensure it is alright

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // if the viewport is smaller than the actual image as defined by the imageWidth and aspectRatio, then we want to 
    // make sure that the ortho is smaller such that the frame we use is bigger than the viewport..
    // and vice versa..
    
    GLfloat x1 = -1.0;
    GLfloat x2 = x1 + 2.0 * (GLfloat)w/(GLfloat)textureWidth;
    GLfloat y1 = -aspectRatio;
    GLfloat y2 = (y1 + 2.0 * aspectRatio * (GLfloat)h/((GLfloat)textureWidth * aspectRatio));


    glOrtho(x1, x2, y1, y2, 1.0, 30.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -4.0);   
}



float* GLImage::make_background(unsigned int width, unsigned int height){
    // makes a nicely coloured background image
    // with distinct corners and edges.. 
    if(!width || !height){
	return(0);
    }
    float* bg = new float[width * height * 3];
    for(uint y=0; y < height; y++){
	for(uint x=0; x < width; x++){
	    bg[(y * width + x) * 3] = 0.5;
	    bg[1 + (y * width + x) * 3] = float(x)/float(width);
	    bg[2 + (y * width + x) * 3] = float(y)/float(width);
	}
    }
    return(bg);
}
