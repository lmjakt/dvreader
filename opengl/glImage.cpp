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

using namespace std;


/*!
  Create a GLImage widget
*/

//const int backgroundWidth = 1024; 
//const int backgroundHeight = 1024;   // the size of the background texture map

// and a couple of things that we'll use for now..
static GLuint texName;  // for the texture
static GLuint extraTex; // an extra texture.. 
static GLuint thirdOne;
static GLuint fourthOne;
//static GLfloat backgroundTexture[1024][1024][3];   // don't think I can use the variables above ?
//static GLubyte backgroundTexture[1024][1024][4];   // don't think I can use the variables above ?
/// anyway, change to GLfloat later and stuff like that.. 

// void makeCheckImage(){
//     int i, j, c;
//     for(i=0; i < backgroundHeight; i++){
// 	for(j=0; j < backgroundWidth; j++){
// 	    c = ((((i&0x8) == 0) ^ ((j&0x8)) == 0)) * 1; // black or white ?
// 	    backgroundTexture[i][j][0] = (GLfloat) c;
// 	    backgroundTexture[i][j][1] = (GLfloat) c;
// 	    backgroundTexture[i][j][2] = (GLfloat) c;
// 	    //backgroundTexture[i][j][3] = (GLfloat) 255;
// 	}
//     }
// }


GLImage::GLImage(unsigned int width, unsigned int height, unsigned int texSize, GLfloat aspRatio, QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
//    xRot = yRot = zRot = 0.0;		// default object rotation
    //    scaleFactor = 1.0;
    // biasFactor = 0.0;
    //   object = 0;
    setMouseTracking(true);
    xo = yo = buttonPressed = 0;
    xscale = yscale = 1;
    aspectRatio = aspRatio;
//    rowSkip = 0;
//    pixelSkip = 0;
    //imageData = 0;
//    rgb_image = 0;    // as these are only pointers, we should probably amalgate them into one, and then just cast.. 
//    extra_image = 0;
//    useRGB = false;   // ugly,, as I said, should change this.. 

    twidth = width;
    theight = height;
    textureSize = texSize;
    textures = 0;   // reassign when initialising.. 
    //   imageWidth = width;
    //imageHeight = height;   // need for resizeGL function in order to work out the appropriate frustum (well should probably use ortho.. but)

    xCross = yCross = 0.0;
    drawCross = false;

    // width and height are used to work out the backgroundHeight and backgroundWidth...
//     backgroundWidth = backgroundHeight = 1;
//     // work out seperatly
//     while(backgroundWidth < width){
// 	backgroundWidth *= 2;
//     }
//     while(backgroundHeight < height){
// 	backgroundHeight *= 2;
//     }
//     cout << "width    is     = " << width << " and  " << height << endl;
//     cout << "backbroundHeight = " << backgroundHeight << "  and  bg width " << backgroundWidth << endl;

//     /// which is ok. Now what we want to do is to check if this is sensible or not.
//     /// allow for up to 2048 pixels by 2048....
//     if(backgroundWidth * backgroundHeight > (2048 * 2048)){
// 	cerr << "Texture with " << backgroundWidth << " * " <<  backgroundHeight << "  pixels is probably a bit too large to handle .. " << endl;
// 	exit(1);
//     }
    backgroundWidth = backgroundHeight = textureSize;
    imageWidth = imageHeight = textureSize;
//    backgroundTexture = new GLfloat[backgroundHeight * backgroundWidth * 3];  //ok.. let's use memset to make that .. grey ? 
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
    if(x > textureSize || y > textureSize){
	cerr << "GLImage::setImage x or y seems to be too large  x :" << x << "  y : " << y  << endl;
	return;
    }
/////////////////////////// important stuff follows from here.. I think.. 

    int texPos = row * twidth + col;
    
//    cout << "GLImage setting data to " << texPos << "th texture with coords " << x << ", " << y << "  and : " << col << "  row " << endl;

    makeCurrent();
    glBindTexture(GL_TEXTURE_2D, textures[texPos]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGB, GL_FLOAT, data);

}

void GLImage::setImage(){
//     if(rgb_image){
// 	memcpy(extra_image, rgb_image, x * y * 3 * sizeof(float));
// //	extra_image = rgb_image;
//     }else{
// 	extra_image = new float[x * y * 3];
// //	memcpy(extra_image, rgb_image, x * y * 3 * sizeof(float));
//     }
    
//     gl_mod_buffer(extra_image, extra_image, x, y, 0.0);

//     rgb_image = data;
//     imageWidth = x;    
    
// //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
// //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backgroundWidth, backgroundHeight, 0, GL_RGB, GL_FLOAT, backgroundTexture);

//     imageHeight = y;
//   // it may be that all we need to do is the following..

/////////////////////////// important stuff follows from here.. I think.. 

//     makeCurrent();
//     for(uint i=0; i < theight; i++){
// 	for(uint j=0; j < twidth; j++){
// 	    glBindTexture(GL_TEXTURE_2D, textures[i * twidth + j]);
// 	    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize, GL_RGB, GL_FLOAT, backgroundImage);
// 	}
//     }

//     makeCurrent();
//     glBindTexture(GL_TEXTURE_2D, texName);
//     glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGB, GL_FLOAT, rgb_image); 

//     glBindTexture(GL_TEXTURE_2D, extraTex);
//     glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGB, GL_FLOAT, extra_image); 
    
//     glBindTexture(GL_TEXTURE_2D, thirdOne);
//     glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGB, GL_FLOAT, rgb_image); 
    
//     glBindTexture(GL_TEXTURE_2D, fourthOne);
//     glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, GL_RGB, GL_FLOAT, extra_image); 
    


//     useRGB = true;   
}


void GLImage::gl_mod_buffer(float* destination, float* source, GLint w, GLint h, GLfloat param){
    makeCurrent();
    cout << "glmod_buffer" << endl;
    glDrawBuffer(GL_BACK);
    glRasterPos2i(0, 0);
    glDrawPixels(w, h, GL_RGB, GL_FLOAT, source);
//    glPixelTransferf(GL_RED_SCALE, param);
    cout << "called glDrawPixels " << endl;
//    glFlush();
    cout << "called glpixeltransfer again.. " << endl;
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGB, GL_FLOAT, destination); 
//    glPixelTransferf(GL_RED_SCALE, 1.0);
    cout << "called glread pixels.. " << endl;
    // is it that simple ? 
}

void GLImage::setMagnification(float m){
  xscale = yscale = m;
  updateGL();
}

void GLImage::transformPos(int x, int y, int& px, int& py, bool setCross){
    px = py = 0;   // in case something doesn't add up at least some kind of reasonable number.. 
    // work out our current coordinates using the offset and the scale values... not sure exactly how, but..
    
    // This is a really long way around, and it is ugly. I suppose that it can be summarised nicely into a single
    // nice equation, but I had a little trouble working out how to do this, so am trying to keep it simple.
    
    // I think this might end up being different if the size of the drawn pixmap is different from the background, 
    // and if the aspectRatio isn't one. (but you never know, it might work..)
    GLfloat frustumPos = -1.0 + ((GLfloat)(2 * x)/(GLfloat)backgroundWidth);
    GLfloat xOrigin = (-1.0 + xo) * xscale;
    GLfloat frustumDistance = frustumPos - xOrigin;
    GLfloat scaledFrustumDistance = frustumDistance / xscale;
    px = backgroundWidth * (scaledFrustumDistance / 2.0);
    
    // y position might be a little bit more complicated, but as long as the aspect ratio is 1, and the whole image is used... no problem.. 
    GLfloat yFrustumPos = -aspectRatio + ((GLfloat)(2 * (height() - y)) / (GLfloat)backgroundHeight);   //
    GLfloat yOrigin = (-aspectRatio + yo) * xscale;
    GLfloat yScaledFrustumDistance = (yFrustumPos - yOrigin) / xscale;
    py = backgroundHeight * (yScaledFrustumDistance / 2.0);
    

    // if we want to draw a cross on the thingy.. 
    if(setCross){ 
	//cout << endl << "frustum Pos : " << frustumPos << "  xOrigin " << xOrigin << "  frustumDistance : " << frustumDistance << "  scaled distance " << scaledFrustumDistance 
	//     << "  pixel coordinate : " << px <<  " and py : " << py <<  endl << endl;
	xCross = scaledFrustumDistance - 1.0;
	yCross = yScaledFrustumDistance - aspectRatio;   // these coordinates will have to be scaled and offset like the box coordinates.. 
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
    if(e->state() == (Qt::ControlButton)){
	// work out our current coordinates using the offset and the scale values... not sure exactly how, but..

	// This is a really long way around, and it is ugly. I suppose that it can be summarised nicely into a single
	// nice equation, but I had a little trouble working out how to do this, so am trying to keep it simple.

	// I think this might end up being different if the size of the drawn pixmap is different from the background, 
	// and if the aspectRatio isn't one. (but you never know, it might work..)

	int px, py;
	transformPos(e->x(), e->y(), px, py, true);

// 	GLfloat frustumPos = -1.0 + ((GLfloat)(2 * e->x())/(GLfloat)backgroundWidth);
// 	GLfloat xOrigin = (-1.0 + xo) * xscale;
// 	GLfloat frustumDistance = frustumPos - xOrigin;
// 	GLfloat scaledFrustumDistance = frustumDistance / xscale;
// 	int px = backgroundWidth * (scaledFrustumDistance / 2.0);

// 	// y position might be a little bit more complicated, but as long as the aspect ratio is 1, and the whole image is used... no problem.. 
// 	GLfloat yFrustumPos = -aspectRatio + ((GLfloat)(2 * (height() - e->y())) / (GLfloat)backgroundHeight);   //
// 	GLfloat yOrigin = (-aspectRatio + yo) * xscale;
// 	GLfloat yScaledFrustumDistance = (yFrustumPos - yOrigin) / xscale;
// 	int py = backgroundHeight * (yScaledFrustumDistance / 2.0);

// 	//cout << endl << "frustum Pos : " << frustumPos << "  xOrigin " << xOrigin << "  frustumDistance : " << frustumDistance << "  scaled distance " << scaledFrustumDistance 
// 	//     << "  pixel coordinate : " << px <<  " and py : " << py <<  endl << endl;
// 	xCross = scaledFrustumDistance - 1.0;
// 	yCross = yScaledFrustumDistance - aspectRatio;   // these coordinates will have to be scaled and offset like the box coordinates.. 


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
    switch(buttonPressed){
	case Qt::LeftButton :
	    xo += 2.0 * ((GLfloat)(e->x() - lastX)) / (xscale * (GLfloat)backgroundWidth) ;
	    yo += aspectRatio * 2.0 * (GLfloat)(lastY - e->y()) / (xscale * (GLfloat)imageWidth * aspectRatio);   // vertical direction is opposite on these things
	    //yo += aspectRatio * 2.0 * (GLfloat)(lastY - e->y()) / (xscale * (GLfloat)backgroundHeight);   // vertical direction is opposite on these things
	    // the offsets need to be divided by the multiplier.. hmm
	    //cout << "backgroundwidth : " << backgroundWidth << "\tbackgroundHeight " << backgroundHeight << "\txo : " << xo << "\t" << yo << endl;
	    emit offSetsSet((int)(xo * (float)backgroundWidth), (int)(yo * (float)backgroundHeight));   // but these should really be divided by the magnification.. hmm 
	    break;
	case Qt::RightButton :
	    // change the magnification multiplier..
	    xscale += (lastY - e->y()) / (GLfloat)(200);
	    //    xscale += (lastY - e->y()) / (GLfloat)(backgroundHeight/2);
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
    if(buttonPressed == Qt::MidButton){
	int x1, x2, y1, y2;
	transformPos(lineStart.x(), lineStart.y(), x1, y1, false);
	transformPos(lineEnd.x(), lineEnd.y(), x2, y2, false);
	emit newLine(x1, y1, x2, y2);   // and whatever..
    }
    buttonPressed = Qt::NoButton; 
}


void GLImage::wheelEvent(QWheelEvent* e){
    emit incrementImage(e->delta() / 120);  // +/- 1  .. should be anyway
    if(e->delta() > 0){
	emit nextImage();
	return;
    }
    emit previousImage();
    // wheels emit -120 / + 120.. so basically.. divide by delta..
    
}

void GLImage::keyPressEvent(QKeyEvent* e){
  int key = e->key();
  //cout << "keyEvent : " << key << endl;

  // note : arrow keys on my stinkpad as follows 
  // up   : 4115
  // down : 4117
  // left : 4114
  // right: 4116
  // 
  // lets use these in order to step through the image...
  switch(key){
  case 4115 :
    emit nextImage();
    break;
  case 4116 :
    emit nextImage();
    break;
  case 4117 :
    emit previousImage();
    break;
  case 4114 :
    emit previousImage();
    break;
  default :
    e->ignore();
  }
  //  e->ignore();  // default 
}

void GLImage::paintGL(){

    // See if we can use a normal painter here..
    glPushAttrib(GL_ALL_ATTRIB_BITS);

//    QPainter p(this);

    glPopAttrib();
    glPushMatrix();
// use a texture to draw on..
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    GLfloat x = (GLfloat)backgroundWidth / (GLfloat)imageWidth;
    GLfloat y1 = -aspectRatio;
    GLfloat y2 = 2.0 * (aspectRatio * (GLfloat)backgroundHeight / (GLfloat)imageHeight) - aspectRatio;

    GLfloat h = y2 - y1;

    if(drawCross){
//	cout << "drawing cross I hope " << endl;
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

    for(uint i=0; i < theight; i++){
	for(uint j=0; j < twidth; j++){
	    GLint texture = textures[i * twidth + j];
	    
	    
	    // ok do the loop here to put all the stuff into appropriate places.. 
	    glBindTexture(GL_TEXTURE_2D, texture);
//	    glBindTexture(GL_TEXTURE_2D, texName);
	    glBegin(GL_QUADS);
	    
	    GLfloat xoo = j * 2 * x;
	    GLfloat yoo = i * h;

	    glTexCoord2f(0, 0); glVertex3f((-x + xo + xoo) * xscale, (y1 + yo + yoo) * xscale, 0);
	    glTexCoord2f(0, 1.0); glVertex3f((-x + xo + xoo) * xscale, (y2 + yo + yoo) * xscale, 0);
	    glTexCoord2f(1.0, 1.0); glVertex3f((x + xo + xoo) * xscale, (y2 + yo + yoo) * xscale, 0);
	    glTexCoord2f(1.0, 0); glVertex3f((x + xo + xoo) * xscale, (y1 + yo + yoo) * xscale, 0);
	    
	    glEnd();
	}
    }


    //   glEnable(GL_TEXTURE_2D);
    // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//     glBindTexture(GL_TEXTURE_2D, extraTex);
//     glBegin(GL_QUADS);

//     glTexCoord2f(0, 0); glVertex3f((x + xo) * xscale, (y1 + yo) * xscale, 0);
//     glTexCoord2f(0, 1.0); glVertex3f((x + xo) * xscale, (y2 + yo) * xscale, 0);
//     glTexCoord2f(1.0, 1.0); glVertex3f((3 * x + xo) * xscale, (y2 + yo) * xscale, 0);
//     glTexCoord2f(1.0, 0); glVertex3f((3 * x + xo) * xscale, (y1 + yo) * xscale, 0);
    
//     glEnd();

// //    glEnable(GL_TEXTURE_2D);
// //    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//     glBindTexture(GL_TEXTURE_2D, thirdOne);
//     glBegin(GL_QUADS);

//     glTexCoord2f(0, 0); glVertex3f((-x + xo) * xscale, (h + y1 + yo) * xscale, 0);
//     glTexCoord2f(0, 1.0); glVertex3f((-x + xo) * xscale, (h + y2 + yo) * xscale, 0);
//     glTexCoord2f(1.0, 1.0); glVertex3f((x + xo) * xscale, (h + y2 + yo) * xscale, 0);
//     glTexCoord2f(1.0, 0); glVertex3f((x + xo) * xscale, (h + y1 + yo) * xscale, 0);
    
//     glEnd();

// //    glEnable(GL_TEXTURE_2D);
// //    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//     glBindTexture(GL_TEXTURE_2D, fourthOne);
//     glBegin(GL_QUADS);

//     glTexCoord2f(0, 0); glVertex3f((x + xo) * xscale, (h + y1 + yo) * xscale, 0);
//     glTexCoord2f(0, 1.0); glVertex3f((x + xo) * xscale, (h + y2 + yo) * xscale, 0);
//     glTexCoord2f(1.0, 1.0); glVertex3f((3 * x + xo) * xscale, (h + y2 + yo) * xscale, 0);
//     glTexCoord2f(1.0, 0); glVertex3f((3 * x + xo) * xscale, (h + y1 + yo) * xscale, 0);
    
//     glEnd();

//     if(glIsEnabled(GL_LIGHTING)){
// 	cout << "lighting is enabled, bugger, that " << endl;
//     }
//     glDisable(GL_LIGHTING);
    // if draw cross, then work out the positions, and draw some lines in the same pos as the crosses...


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
    backgroundImage = make_background(textureSize);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    //makeCheckImage();   // define an array that we can use as the initial texture. We probably don't need to remember this, but.. 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // note if we use RGBA or we use floats we shouldn't need to call this.. (but for now the example.._)
    textures = new GLuint[theight * twidth];
    glGenTextures(twidth * theight, textures);    // makes the required number of textures.. 
     for(unsigned int i=0; i < theight; i++){
	for(unsigned int j=0; j < twidth; j++){
	    glBindTexture(GL_TEXTURE_2D, textures[i * twidth + j]);
	    //glGenTextures(1, &texName);
//	    glBindTexture(GL_TEXTURE_2D, texName);
	    
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backgroundWidth, backgroundHeight, 0, GL_RGB, GL_FLOAT, backgroundImage);
	    GLint resWidth;
	    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &resWidth);
	    cout << "generate texture at " << j << "," << i << "\tresulting width : " << resWidth << endl;

	}
    }
    ////////////////// and generate an extra texture.. 

//     delete backgroundImage;
//     backgroundImage = 0;


}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLImage::resizeGL( int w, int h )
{
    //glViewport( 0, 0, imageWidth, (GLint)((GLfloat)imageWidth * aspectRatio)); // which should be ok. 

    glViewport(0, 0, w, h);  // but then we have to change the width of the frustum or the orthogonal to ensure it is alright

//    glViewport( 0, 0, backgroundWidth, backgroundHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    gluPerspective(60.0, (GLfloat) w/ (GLfloat) h, 1.0, 30.0);
    //glFrustum(-1.0, 1.0, -aspectRatio, aspectRatio, 1.0, 30.0);   // 
//    glFrustum(-1.0, 1.0, -aspectRatio, aspectRatio, 1.0, 30.0);   // 

    // if the viewport is smaller than the actual image as defined by the imageWidth and aspectRatio, then we want to 
    // make sure that the ortho is smaller such that the frame we use is bigger than the viewport..
    // and vice versa..
    
    GLfloat x1 = -1.0;
    GLfloat x2 = x1 + 2.0 * (GLfloat)w/(GLfloat)backgroundWidth;
    GLfloat y1 = -aspectRatio;
    GLfloat y2 = y1 + 2.0 * aspectRatio * (GLfloat)h/((GLfloat)imageWidth * aspectRatio);

//    GLfloat mh = (GLfloat)h/(GLfloat)(imageWidth);

    //glOrtho(-mw, +mw, -mh, +mh, 1.0, 30.0);

    glOrtho(x1, x2, y1, y2, 1.0, 30.0);
//    glOrtho(x1, x2, y1, y2, 0.1, 30.0);
//    glOrtho(-1.0, 1.0, -aspectRatio, aspectRatio, 1.0, 30.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -4.0);   
}



float* GLImage::make_background(unsigned int width){
    // makes a nicely coloured background image
    // with distinct corners and edges.. 
    if(!width){
	return(0);
    }
    float* bg = new float[width * width * 3];
    for(uint y=0; y < width; y++){
	for(uint x=0; x < width; x++){
	    bg[(y * width + x) * 3] = 0.5;
	    bg[1 + (y * width + x) * 3] = float(x)/float(width);
	    bg[2 + (y * width + x) * 3] = float(y)/float(width);
	}
//	cout << endl;
    }
    cout << endl;
    return(bg);
}
	    
