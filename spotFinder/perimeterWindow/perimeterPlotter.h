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

#ifndef PERIMETERPLOTTER_H
#define PERIMETERPLOTTER_H

#include "../../dataStructs.h"
#include <QGLWidget>
#include <QList>
#include <deque>
#include <QPolygon>
#include <q3pointarray.h>
#include <QColor>
//#include <qcolor.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <vector>

struct outlineData {
    QColor c;
//    QPointArray points;
    std::vector<twoDPoint> points;
    int minX, maxX;
    int minY, maxY;  // this is somehow necessary to know..
    outlineData(){
	c = QColor(255, 255, 255);
	minX = maxX = minY = maxY = 0;
    }
    outlineData(int minx, int maxx, int miny, int maxy){
	minX = minx;
	maxX = maxx;
	minY = miny;
	maxY = maxy;
	c = QColor(255, 255, 255);
    }
};

class PerimeterPlotter : public QGLWidget 
{   
    Q_OBJECT
	public :
	PerimeterPlotter(int texSize, QWidget* parent=0, const char* name=0);
    ~PerimeterPlotter(){
	delete background;
//	if(foreground){
//	    delete foreground;
//	}
    }
    
    public slots :
    void setLines(std::vector<outlineData> data);
    void setPoints(std::vector<twoDPoint> p);
    void setHighlights(std::map<twoDPoint, QColor> qpc);
    void setPath(std::vector<twoDPoint> pts);
    void setPolyPath(std::vector<twoDPoint> pts);
    void setScale(double x, double y);
    void setScale(double s);
    void setBackground(float* data, unsigned int w, unsigned int h);
    
    private :
	void paintEvent(QPaintEvent* e);
    double xScale, yScale;
//    std::deque<QPolygon> userLines;
    QList<QPolygon> userLines;
    bool drawingLines;
    int textureSize;
    GLuint texture;  // the id of the texture number set up in thingy
    float* background;
    float* foreground;
    int foreground_w, foreground_h;

    protected :
	void initializeGL();
    void resizeGL(int width, int height);
    // but not paintGL as we'll do all the painting in the thingy.. 
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void wheelEvent(QWheelEvent* e);
    
    std::vector<outlineData> outlines;
    std::vector<twoDPoint> points;   // draw these in some manner or other.. 
    std::map<twoDPoint, QColor> highlights;  // highlight these in some way or other
    std::vector<twoDPoint> path;         // quick and dirty pathdrawing thing.. 
    std::vector<twoDPoint> polyPath;     // draw as a-b, c-d, rather than a-b-c-d
    std::vector<QColor> colorMap;       // set up a colour map at the beginning.. 

    signals :
	void changeSet(int);
    void changePerimeter(int);
    void splitPerimeter(QList<QPolygon>);  // split the current perimeter using these points.. 
    void drawSelected();
    void mousePos(int, int);
};

#endif
