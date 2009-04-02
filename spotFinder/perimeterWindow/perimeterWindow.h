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

#ifndef PERIMETERWINDOW_H
#define PERIMETERWINDOW_H

#include "perimeterPlotter.h"
#include "../../distanceMapper/objectComparer.h"
#include "../../distanceMapper/compareController.h"
#include "../perimeter.h"
//#include "../../deltaViewer.h"
#include <vector>
#include <qspinbox.h>
#include <qlabel.h>
#include <QCheckBox>

class DeltaViewer;

class PerimeterWindow : public QWidget
{
    Q_OBJECT
	public :
	PerimeterWindow(DeltaViewer* dv, QWidget* parent=0, const char* name=0);
    ~PerimeterWindow(){
	delete plotterBackground;
    }
    void setPerimeters(std::vector<PerimeterSet> perimeters, float wl, int sno);

    signals :
	void perimetersFinalised(std::vector<PerimeterSet>, float, int);
    
    private :
	// giving everyone a piece of the FileSet might be a bit dodgy, but it's convenient
	DeltaViewer* deltaViewer;

    PerimeterPlotter* plotter;
    CompareController* compareController;
    ObjectComparer* perimeterComparer;
    std::vector<PerimeterSet> persets;
    QSpinBox* setSelector; // selects the set
    QSpinBox* perSelector; // selects which perimeter to include.. 

    QLabel* setLabel;
    QLabel* perLabel;

    QSpinBox* scaleSelector;  // set the scale of the drawing if it doesn't quite fit
    QCheckBox* pixelCheck;
 

   // and then some slots ..
    int sliceNo;      
    float waveLength;
    int currentSet, currentPerimeter;  // if either of these change we may need to update the thingy.. 

    // these refer to the current max/min of the displayed area. Note that we don't set this
    // if we are displaying things from more than one area.. 
    int origin_x, origin_y, perimeter_w, perimeter_h;
    int plotterTextureSize;
    float* plotterBackground;

    // some function to create continuous lines from QPolygons..
    std::vector<int> boundaryFromPolygon(QPolygon poly, int xo, int yo, int gw);

    private slots :
	void redrawPerimeters(int value);  // value actually ignored, poll the different selectors.. 
    void newSet(int sno);   // set the value of the thingy..
    void setScale(int scale);
    void drawBackground();
    void comparePerimeters(float, float);
    void changeSet(int delta);
    void changePerimeter(int delta);
    void splitPerimeter(QList<QPolygon> polys);
    void drawSelected();  // draws the selected thing for a given perimeter set
    void acceptPerimeters();
    

};
	
#endif
