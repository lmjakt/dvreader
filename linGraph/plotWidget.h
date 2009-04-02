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

#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <vector>
#include <qwidget.h>
#include <qcolor.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include "slider.h"
#include <map>

class PlotWidget : public QWidget
{
    Q_OBJECT

	public :
	PlotWidget(QWidget* parent=0, const char* name=0);
    ~PlotWidget();  // destroy the slider bar..
    
    public slots:
	void setValues(std::vector<std::vector<float> > v, std::vector<int> m, int LMargin, int RMargin, float MinY=0, float MaxY=0);
    void setValues(std::vector<std::vector<float> > v);
    void setColors(std::vector<QColor> Colors);
    void setMinPeakValue(unsigned int id, float mpv);
    void setMaxEdgeValue(unsigned int id, float mev);
    void setScale(unsigned int id, float s);
    void setPeaks(std::map<int, std::vector<int> > Peaks);
    void clearPeaks();
    // since the 

    signals :
	void incrementImage(int);    // move the position of the slice in one direction.. 

    private :
	void paintEvent(QPaintEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);
    int lastX, lastY;  // for tracking mouse movement.
    Slider* rangeSelector;
    std::vector<std::vector<float> > values;
    std::vector<QColor> colors;    // for each line draw in Color .. 
    std::vector<float> minPeakValues;
    std::vector<float> maxEdgeValues;     // two paramaters (normally set to 0 and 1)... 
    std::vector<float> scales;           
    std::vector<int> marks;   // draw a vertical line at the marks..
    int lMargin, rMargin;  // some margins that we consider to be not interesting or something..
    QColor backgroundColor;  // but this is black.. 
    float minY, maxY;
    int maxX;                // so we can have arrays of different lengths. 
    int sliderState;         // state of slider, 0 -not moving, 1 low range moving, 2 high range moving, 3, both moving.. (so this could be done with a bitwise)
    
    // some stuff for peak detection and so on ..
    std::map<int, std::vector<int> > peaks;

};

#endif
