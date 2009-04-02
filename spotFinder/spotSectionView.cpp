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

#include "spotSectionView.h"
#include <qpainter.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <iostream>

using namespace std;

SpotSectionView::SpotSectionView(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    // background is black.. 
    setEraseColor(QColor(0, 0, 0));
    sp_width = 0;
    sp_height = 0;
    // we don't care about size, or anything else
    // that is handled by whomever handles us..
    
}

void SpotSectionView::setView(vector<QColor> v, int w, int h){
    // first make sure that w * h = v.size()
    if(v.size() != w * h){
	cerr << "vector size " << v.size() << " doesn't fit : " << w << " * " << h << endl;
	return;
    }
    values = v;
    sp_width = w;
    sp_height = h;

    repaint();
}

void SpotSectionView::paintEvent(QPaintEvent* e){
    if(!values.size()){
	return;
    }
    int sq_width = width() / sp_width;
    int sq_height = height() / sp_height;
    QPainter p(this);
    p.setPen(Qt::NoPen);
    for(int y=0; y < sp_height; y++){
	for(int x=0; x < sp_width; x++){
	    int cp = y * sp_width + x;
	    p.setBrush(values[cp]);
	    p.drawRect(x * sq_width, y * sq_height, sq_width, sq_height);
	}
    }
}

// and that's all really. 
