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

#ifndef GLIMAGE_H
#define GLIMAGE_H

#include <QGLWidget>
#include <qpoint.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <vector>

class GLImage : public QGLWidget
{
    Q_OBJECT

public:

    GLImage(unsigned int width, unsigned int height, unsigned int texSize,  GLfloat aspRatio=1.0, QWidget* parent=0, const char* name=0 );
    GLImage(unsigned int width, unsigned int height, unsigned int texWidth,  unsigned int texHeight,
	    GLfloat aspRatio=1.0, QWidget* parent=0, const char* name=0 );
    ~GLImage();

public slots:

 void setImage(float* data, int x, int y, int col, int row);           // using an rgb coordinate system.. 
 void setBigImage(float* data, int source_x, int source_y, 
	       int width, int height);
 void clearTextures();
 void setMagnification(float m);
 void resetMagnification(){
     xscale = 1.0;
     updateGL();
     emit magnificationSet(1.0);
 }
 void resetOffsets(){
     xo = yo = 0;
     updateGL();
     emit offSetsSet(0, 0);
 }
 // void setScaleAndBias(float s, float b);

protected:

    void initializeGL();
    void paintGL();
    void resizeGL( int w, int h );
    void mouseMoveEvent(QMouseEvent* e);  // so we can change position.
    void mousePressEvent(QMouseEvent* e); // set lastx and lasty
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);  // toggle the draw X property.. 
    void wheelEvent(QWheelEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void transformPos(int x, int y, int& px, int& py, bool setCross=true);   // take widget positions x and y, and transform those to picture coordinates px and py

    //virtual GLuint 	makeObject();

private:
    
    void gl_mod_buffer(float* destination, float* source, GLint w, GLint h, GLfloat param);  // try to use gldrawpixels and glreadpixels to modify a data set
    float* make_background(unsigned int width, unsigned int height);   // make a  square of some sort .. 
    bool mapImageToTexture(float* image_data, int source_x, int source_y,
			   int source_w, int source_h, int tex_x, int tex_y, GLuint texture);

    //    bool animation;
    //    GLuint object;
    GLuint* textures;
    GLint twidth, theight;   // the number of textures, not the
    //    GLint textureSize;     // the size of the texture.. 
    GLint textureWidth;
    GLint textureHeight;
    std::vector<float*> images;   // the images that we want to map to the different areas.. 
    float* backgroundImage;       // a single image that we can use to set things up.. 
    
//    GLfloat xRot, yRot, zRot;
    GLfloat xo, yo;    // x and y origin.. before magnification ! 
    GLfloat xscale, yscale;
//    GLint rowSkip, pixelSkip;  // to allow movement of the image (simulated)
    int lastX, lastY, buttonPressed;
//    unsigned char* imageData;  // assume 8 bit greyscale.. basically.. 
//    float* rgb_image;          // image in floats.. somewhat different.. 
//    float* extra_image;
//    bool useRGB;               // whether or not we use RGB..would probably be better to replace with a state variable. enum.. 

    // It seems that the below are always equal to textureWidth and textureHeight, 
    // so I should probably get rid of them.
    //GLint imageWidth, imageHeight;
    //GLint backgroundWidth, backgroundHeight;

    GLfloat xCross, yCross;   // draw a cross at these position if something is true..
    bool drawCross;           // 
    //   GLfloat* backgroundTexture;
    GLfloat aspectRatio;   // this is the ratio of the aspect of the image that we want to put there. Expressed as y/x
    //GLfloat scaleFactor, biasFactor;  // for the scaling of colours.. 
    //QTimer* timer;
    //QImage tex1;
    QPoint lineStart;
    QPoint lineEnd;    // if we draw a line on top of stuff.. 
    
    signals :
	void nextImage();
    void previousImage();
    void firstImage();
    void lastImage();
    void incrementImage(int);   // change image pos by int.. 
    void offSetsSet(int, int);  // tell owners I've set the offset.
    void magnificationSet(float);  // tell owner that the magnification has increased.. 
    void newPos(int, int);       // instruct someone how to slice the cube.. 
    void newLine(int, int, int, int);  // x1, y1,, x2, y2
    void mousePos(int, int);  // the translated mouse Position.. 
};


#endif // GLTEXOBJ_H
