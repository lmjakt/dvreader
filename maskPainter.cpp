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

#include "maskPainter.h"
#include <qpainter.h>
#include <qcolor.h>
//Added by qt3to4:
#include <QPaintEvent>
#include <string.h>

MaskPainter::MaskPainter(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    setEraseColor(QColor(0, 0, 0));
    mask = 0;
}

void MaskPainter::setData(char* m, int w, int h){
    if(mask){
	delete mask;
    }
    mask = new char[w * h];
    memcpy((void*)mask, (void*)m, w * h * sizeof(char));
    m_width = w;
    m_height = h;
    update();
}

void MaskPainter::paintEvent(QPaintEvent* e){
    QPainter p(this);
    p.setPen(QPen(QColor(255, 255, 255), 1));
    for(uint y=0; y < m_height; ++y){
	for(uint x=0; x < m_width; ++x){
	    if(mask[y * m_width + x]){
		p.drawPoint(x, height() - y);
	    }
	}
		
    }
}
