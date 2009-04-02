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

#include <GL/gl.h>
#include <GL/glut.h>
#include "perimeterPlotter.h"
#include <qpainter.h>
#include <qmatrix.h>
#include <qpen.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <QFont>
#include <QGLFormat>
#include <QGLContext>
#include <iostream>


using namespace std;

PerimeterPlotter::PerimeterPlotter(int texSize, QWidget* parent, const char* name)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent, name)
{
//    setEraseColor(QColor(0, 0, 0));  // since we'll do the painter events after the openGL event.. 
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_NoSystemBackground);
    xScale = 1.0;
    yScale = 1.0;
    textureSize = texSize;  // shouldn't need much more than that I think.. 
//    textureSize = 1024;  // shouldn't need much more than that I think.. 
    background = foreground = 0;
    drawingLines = false;

    // set up a map of QColors ..
    int mapSize = 100;
    colorMap.reserve(mapSize);
    int colors[] = {200, 150, 0};  // just for counting (in order to change one each time)
    int inc[] = {120, 95, 180, 250, 120};
    int i = 0;
    while(colorMap.size() < mapSize){
	colors[i % 3] += inc[i % 5];
	int r = colors[0] % 255;
	int g = colors[1] % 255;
	int b = colors[2] % 255;
	cout << colors[0] << "," << colors[1] << "," << colors[2] << "  -->  " << r << "," << g << "," << b << endl;
	if(r + g + b > 150)
	    colorMap.push_back(QColor(r, g, b));
	++i;
    }

    QGLFormat format;
    QGLContext* context = new QGLContext(format, this);
    context->create();
    
}

void PerimeterPlotter::setLines(vector<outlineData> data){
    cout << "set data user lines size is " << userLines.size() << endl;
    userLines.clear();
    outlines = data;
//    repaint(); // let the calling widget decide on this.. 
}

void PerimeterPlotter::setPoints(vector<twoDPoint> p){
    points = p;
}

void PerimeterPlotter::setHighlights(map<twoDPoint, QColor> qpc){
    highlights = qpc;
}

void PerimeterPlotter::setPath(vector<twoDPoint> pts){
    path = pts;
}

void PerimeterPlotter::setPolyPath(vector<twoDPoint> pts){
    polyPath = pts;
}

void PerimeterPlotter::setScale(double x, double y){
    xScale = x;
    yScale = y;
}

void PerimeterPlotter::setScale(double s){
    xScale = yScale = s;
    update();
}

void PerimeterPlotter::mousePressEvent(QMouseEvent* e){
    emit mousePos((int)((double)e->x()/xScale), (int)(double((height() - e->y()))/yScale) );
    // basically let's just append userLines
    cout << "mousePressEvent received at " << e->x() << "," << e->y() << endl
	 << "after scaling " << (int)((double)e->x()/xScale) << "," <<  (int)(double(height() - e->y())/yScale) << endl;
    QPolygon p;
    switch(e->button()){
	case Qt::LeftButton :
	    userLines.push_back(p);
	    drawingLines = true;
	    break;
	case Qt::RightButton :
	    // do something
	    emit changeSet(1);
	    break;
	case Qt::MidButton :
	    // do somethin
	    emit changeSet(-1);
	    break;
	default :
	    cerr << "unknown mouse button" << endl;
    }
}

void PerimeterPlotter::mouseReleaseEvent(QMouseEvent* e){
    drawingLines = false;
    if(userLines.size() && userLines.back().size() < 2)
	userLines.pop_back();
}

void PerimeterPlotter::mouseMoveEvent(QMouseEvent* e){
    if(!drawingLines){
	return;
    }
//    cout << "mouseMoveEvent received at " << e->x() << "," << e->y() << endl;
    userLines.back().push_back(QPoint((int)((double)e->x()/xScale), (int)double((height() - e->y()))/yScale ) );
//    update();
    repaint();
}

void PerimeterPlotter::keyPressEvent(QKeyEvent* e){
    switch(e->key()){
	case Qt::Key_Space :
	    cout << "space bar hit emitting splitPerimeter userLines size is " << userLines.size()  << endl;
	    emit splitPerimeter(userLines);
	    break;
	case Qt::Key_P :
	    cout << "key P hit " << endl;
	    break;
	case Qt::Key_Right :
	    emit changeSet(1);
	    break;
	case Qt::Key_Left :
	    emit changeSet(-1);
	    break;
	case Qt::Key_Up :
	    emit changePerimeter(1);
	    break;
	case Qt::Key_Down :
	    emit changePerimeter(-1);
	    break;
	case Qt::Key_S :
	    emit drawSelected();
	    break;
	default :
	    cout << "unspecified key code hit" << endl;
	    e->ignore();
    }
}

void PerimeterPlotter::wheelEvent(QWheelEvent* e){
    cout << "wheel event received delta is : " << e->delta() << endl;
    if(e->delta() > 0){
	emit changePerimeter(1);
	return;
    }
    changePerimeter(-1);
}

void PerimeterPlotter::initializeGL(){
    // may not need to do anything here.. 
    makeCurrent();
    // lets initialise a couple of things..
    background = new float[textureSize * textureSize * 3];

    memset((void*)background, 0, textureSize * textureSize * 3 * sizeof(float));
//     for(int y=0; y < textureSize; ++y){
// 	for(int x=0; x < textureSize; ++x){
// 	    background[3 * (y * textureSize +x)] = (float)y / (float)textureSize;
// 	    background[3 * (y * textureSize +x) + 1 ]= float(x % 256) / 256.0;
// 	    background[3 * (y * textureSize +x) + 2] = float(y % 256) / 256.0;
// 	}
//     }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // note if we use RGBA or we use floats we shouldn't need to call this.. (but for now the example.._)
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0, GL_RGB, GL_FLOAT, background);

    glBindTexture(GL_TEXTURE_2D, texture);

//    delete background;  // ?can I do this ? 
}

void PerimeterPlotter::resizeGL(int width, int height){
    makeCurrent();
    glViewport(0, 0, width, height);
    // do others in paintEvent ?
}

void PerimeterPlotter::paintEvent(QPaintEvent* e){
    makeCurrent();
    // very simple..
    QPainter p;
    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing);
    QMatrix m;
    m.setMatrix(1, 0, 0, -1.0, 0, 0.0);
    p.save();
    p.setWorldMatrix(m);


    //////////////////////// BEGIN openGL STUFF
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glClearColor(0.0, 0.1, 0.2, 0.0);
//    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width(), height());
    glOrtho(0, width(), 0, height(), 1.0, 30.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // and here we do the gl painting stuff.. 
    int margin = 40;
    int z = 0;

    // need to set the pen or not ?? 
    glScalef(xScale, -yScale, 1.0);
    glTranslatef(0.0, ((float)-height())/yScale, 0.0);

    ///////////////////////////


    // set up the pixel thingy..
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3i(0, 0, z-1);
    glTexCoord2f(0,1); glVertex3i(0, textureSize, z-1);
    glTexCoord2f(1,1); glVertex3i(textureSize, textureSize, z-1);
    glTexCoord2f(1,0); glVertex3i(textureSize, 0, z-1);
    glEnd();
    glPopAttrib();

    
    glPopAttrib();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    ////////////////////// END openGL STUFF 


    p.translate(0.0, -(double)height());
    p.scale(xScale, yScale);
    p.setBrush(Qt::NoBrush);
    int penWidth = 0;
    for(uint i=0; i < outlines.size(); ++i){
	p.setPen(QPen(outlines[i].c, penWidth));
	for(uint j=0; j < outlines[i].points.size(); ++j){
	    int px, py;  // the previous points..
	    if(j){
		px = outlines[i].points[j-1].x;
		py = outlines[i].points[j-1].y;
	    }else{
		px = outlines[i].points.back().x;
		py = outlines[i].points.back().y;
	    }
	    p.drawLine(outlines[i].points[j].x, outlines[i].points[j].y, px, py);
	}
    }
    
    // draw the path if any..
    p.setPen(QPen(QColor(255, 0, 255), 2));
    for(uint i=1; i < path.size(); ++i){
	p.drawLine(path[i-1].x, path[i-1].y, path[i].x, path[i].y);
    }

    // and also draw the poly path..
    for(uint i=0; i < polyPath.size(); i += 2){
	p.drawLine(polyPath[i].x, polyPath[i].y, polyPath[i+1].x, polyPath[i+1].y);
    }

    // and draw a line next to the thingy..
    p.setPen(QPen(QColor(0, 255, 255), 0));

    for(uint i=0; i < userLines.size(); ++i){
	p.drawPolyline(userLines[i]);
    }

    // and then the highlights
    for(map<twoDPoint, QColor>::iterator it=highlights.begin(); it != highlights.end(); it++){
	p.setPen(QPen(it->second, 3));
	p.drawRect(it->first.x - 4, it->first.y - 4, 9, 9);
//	cout << "Drawing HIGHLIGHT at " << it->first.x << ", " << it->first.y << endl;
    }

    p.setPen(QPen(QColor(255, 255, 255)));
    // on top of everything draw the points
    for(uint i=0; i < points.size(); ++i){
	if(points[i].id == -1){
	    p.setPen(QPen(QColor(255, 255, 255)));
	}else{
	    p.setPen(QPen(colorMap[points[i].id % colorMap.size()]));
	}
	p.drawRect(points[i].x - 1, points[i].y - 1, 3, 3);
    }
    
//     for(uint i=0; i < colorMap.size(); i++){
// 	int w = 10;
// 	p.setBrush(colorMap[i]);
// 	p.setPen(Qt::NoPen);
// 	p.drawRect(i * w, i * w, w, w);
//     }

//     if(userLines.size()){
// //	p.drawPoints(userLines.back());
// 	p.drawPolyline(userLines.back());
//     }

    //   p.setFont(f1);
//    p.drawText(20, 20, "Text drawn at 20, 20 I hope it won't be upside down");
//    p.setFont(f2);
//    p.drawText(300, 300, "Text drawn at 300, 300.. in font f2 should be helvetica");

    p.restore();
    p.end();
}

void PerimeterPlotter::setBackground(float* data, unsigned int w, unsigned int h){
    if(w > textureSize || h > textureSize){
	cerr << "width or height appear to be larger than texture size" << endl;
	return;
    }
//     if(foreground){
// 	delete foreground;
//     }
    foreground = data;
    foreground_w = w;
    foreground_h = h;
    // first make a background image to clear with..
//    float* bg = new float[textureSize * textureSize * 3];
//    memset((void*)bg, 0, textureSize * textureSize * 3 * sizeof(float));

    makeCurrent();

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize, GL_RGB, GL_FLOAT, background);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, foreground);


//    glFlush();
//    delete bg;
}
