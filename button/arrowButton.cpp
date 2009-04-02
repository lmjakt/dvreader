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

#include "arrowButton.h"
#include <q3pointarray.h>
#include <qregion.h>
#include <iostream>

using namespace std;

ArrowButton::ArrowButton(int angle, QWidget* parent, const char* name)
    : Q3Button(parent, name)
{
    Angle = angle;
}

QSize ArrowButton::sizeHint() const
{
    return(QSize(30, 30));
}

void ArrowButton::drawButton(QPainter* p){
    // first find out the size of the drawing area..
    //cout << "drawButton called " << endl;
    int w = width();
    int h = height();
    int l = w < h ? w : h;
    
    double dw = double(w);
    double dh = double(h);
    double dl = double(l);

    QColor buttonColor(125, 125, 200);
    QColor edgeColor(50, 50, 50); 
    if(isDown()){
	buttonColor = QColor(75, 75, 125);
	edgeColor = QColor(100, 100, 100);
    }
	

    p->setPen(Qt::NoPen);
    p->translate(double(w/2), double(h/2));
    p->scale(dw/dl, dh/dl);
    p->rotate((double)Angle);
    Q3PointArray r;
    r.putPoints(0, 4,  0,-l/2,  -l/2,l/2,  l/2, l/2,  0, -l/2);
    
    r.translate(-1, -1);
    
    p->setBrush(edgeColor);
    p->drawPolygon(r);
    r.translate(1, 1);
    p->setBrush(buttonColor);
    p->drawPolygon(r);
}

void ArrowButton::drawButtonLabel(QPainter* p){
    // not sure what to do ehre
    cout << "drawButtonLabel called " << endl;
}

