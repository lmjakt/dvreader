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

#ifndef SLIDER_H
#define SLIDER_H

#include <qrect.h>
#include <qpainter.h>
#include <qcolor.h>

class Slider {
    public :
	Slider(QRect r, int Min, int Max);   // min and max values. (I suppose I could use floats, but maybe that should be a subclass ?)
    ~Slider();          // but shouldn't need to do much.
    
    void setInnerColor(QColor c);
    void setOuterColor(QColor c);   // color to draw the outline with.
    void draw(QPainter* p);           // implements a drawing..
    
    // and some checks that might want to be done..
    bool innerContains(int x, int y);
    bool outerContains(int x, int y);
    void setLimits(int Low, int High);     // the scaled values
    void setPixelLimits(int Low, int High); // change the scaled values by calculating positions.. 
    void adjustLimits(int dl, int dh);     // where dh and dl are delta high and delta low respectively.. in pixels !! 
    void setPosition(QRect r);             // change the area where we draw ourselves.. 
    void setRange(int Min, int Max);
    int begin();
    int end();

    private :
	QRect innerBox;
    QRect outerBox;
    int min, max;   // max and min values.. represented
    int high, low;  // the selected range
    QColor innerColor;   // default to nice purple..
    QColor outerColor;   // default to white..
};


#endif
