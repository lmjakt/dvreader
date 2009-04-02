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

#include "dropButton.h"
#include <iostream>

using namespace std;

DropButton::DropButton(DropRepresentation type, int init_size, QWidget* parent, const char* name)
    : Q3Button(parent, name)
{

    initialSize = init_size;
    setToggleButton(true);
    dropType = type;
    setOn(false);
    setFixedWidth(init_size);
    setFixedHeight(init_size);
}

QSize DropButton::sizeHint() const
{
    return(QSize(initialSize, initialSize));   // but I should change this to something else, really..
}

void DropButton::drawButton(QPainter* p){
    int w = width();
    int h = height();
    
    p->setPen(Qt::NoPen);
    if(isOn()){
	p->setBrush(QColor(219, 225, 255));
    }else{
	p->setBrush(QColor(199, 205, 235));
    }
    p->drawRect(0, 0, w, h);   // fill in all of the background. (this is pretty ugly, but I can't be bothered..)
 
    int r = 90;
    int g = 98;
    int b = 71;

    QColor linColor = QColor(90, 98, 71);

    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(linColor, 1));


    int rx, ry;  // for thingy..
    
    int indent = 1;
    int minDim = w > h ? h : w;
    int c_delta = 2 * 70 / (minDim - 2);
    int cr = r + 20;
    int cg = g + 20;
    int cb = b + 20;

    // and then draw depending on the type that we are..
    switch(dropType){
	case DOT :
	    p->setBrush(linColor);
	    p->drawRect(w/2 - 1, h/2 - 1, 2, 2);
	    break;
	case CIRCLE :
	    p->drawEllipse(1, 1, w-2, h-2);
	    break;
	case FIVE_POINTS :
	    p->drawPoint(1, 1);
	    p->drawPoint(w-1, 1);
	    p->drawPoint(1, h-1);
	    p->drawPoint(w-1, h-1);
	    p->drawRect(w/2 - 1, h/2 - 1, 2, 2);
	    break;
	case SQUARE :
	    p->drawRect(2, 2, w-4, h-4);
	    break;
	case BY_VALUE :
	    // hmm, this is a bit more complicated.. 
	    rx = 1;
	    ry = 1;
	    while(rx < w/2 && ry < h/2){
		p->setPen(QPen(QColor(cr, cg, cb), 1));
		p->drawEllipse(rx, ry, w - (rx * 2), h - (ry * 2));
		cr -= c_delta;
		cg -= c_delta;
		cb -= c_delta;
		++rx;
		++ry;
	    }
	    break;
	case NO_REP :
	    // don't draw anything.. 
	    break;
	default :
	    cerr << "Unknown drop type, can't draw button " << endl;
    }
}



void DropButton::drawButtonLabel(QPainter* p){
    // not sure what to do ehre
    cout << "drawButtonLabel called " << endl;
}


