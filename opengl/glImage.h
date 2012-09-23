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

#include <Qt>
#include <QGLWidget>
#include <qpoint.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <vector>

struct texture_overlap {
  int cp_w;
  int cp_h;
  int s_bx; int s_by;
  int t_bx; int t_by;
  texture_overlap(){
    cp_w = cp_h = s_bx = s_by = t_bx = t_by = 0;
  }
};

class GLImage : public QGLWidget
{
    Q_OBJECT
      
      
    public:

    GLImage(unsigned int width, unsigned int height, unsigned int texSize,  GLfloat aspRatio=1.0, QWidget* parent=0, const char* name=0 );
    GLImage(unsigned int width, unsigned int height, unsigned int texWidth,  unsigned int texHeight,
	    GLfloat aspRatio=1.0, QWidget* parent=0, const char* name=0 );
    ~GLImage();
    void currentMousePos(int& x, int& y);
    int currentMouseX();
    int currentMouseY();
    enum ViewState { VIEW, DRAW };

public slots:

 void setImage(float* data, int x, int y, int col, int row);           // using an rgb coordinate system.. 
 void setBigImage(float* data, int source_x, int source_y, 
	       int width, int height);
 void setBigOverlay(unsigned char* data, int source_x, int source_y,
		    int width, int height);
 void clearTextures();
 void setMagnification(float m);
 void setPosition(int x, int y);
 void setViewState(ViewState vstate);
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
 void currentView(int& x, int& y, int& w, int& h);

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
    ViewState viewingState;
    void gl_mod_buffer(float* destination, float* source, GLint w, GLint h, GLfloat param);  // try to use gldrawpixels and glreadpixels to modify a data set
    float* make_background(unsigned int width, unsigned int height);   // make a  square of some sort .. 
    bool mapImageToTexture(float* image_data, int source_x, int source_y,
			   int source_w, int source_h, int tex_x, int tex_y, GLuint texture);
    bool mapOverlayToTexture(unsigned char* image_data, int source_x, int source_y,
			     int source_w, int source_h, int tex_x, int tex_y, GLuint texture);
    texture_overlap sourceTextureOverlap(int source_x, int source_y, int source_w, int source_h,
					 int tex_x, int tex_y);
    //void generate_overlay_textures();
    
    GLuint* textures;
    GLuint* overlay_textures;  // RGBA GL_UNSIGNED_BYTE overlay textures (that can be switched on or off by some means)
    bool showOverlay;
    GLint twidth, theight;   // the number of textures, not the
    GLint textureWidth;
    GLint textureHeight;
    std::vector<float*> images;   // the images that we want to map to the different areas.. 
    float* backgroundImage;       // a single image that we can use to set things up.. 
    
    GLfloat xo, yo;    // x and y origin.. before magnification ! 
    GLfloat xscale, yscale;  // yscale isn't actually used at the moment.
    int lastX, lastY;
    Qt::MouseButton buttonPressed;
    int currentX, currentY;  // the current mouse position on the image (rather than the widget position).

    GLfloat xCross, yCross;   // draw a cross at these position if something is true..
    bool drawCross;           // 
    GLfloat aspectRatio;   // this is the ratio of the aspect of the image that we want to put there. Expressed as y/x

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
    // The following signals are only sent if the ViewState is not VIEW,
    // and use the transformed positions (i.e. image based rather than the widget based).
    void mousePressed(QPoint p, Qt::MouseButton button);
    void mouseMoved(QPoint p, Qt::MouseButton button);
    void mouseReleased(QPoint p, Qt::MouseButton);   

    // emit key events..
    void keyPressed(QKeyEvent* e);
};


#endif // GLTEXOBJ_H
