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

#include "flatView.h"
#include <iostream>
#include <qpainter.h>
#include <qstring.h>
//Added by qt3to4:
#include <QPaintEvent>

using namespace std;

FlatView::FlatView(int w, int h, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    imageWidth = w;
    imageHeight = h;
    resize(imageWidth, imageHeight);
    setEraseColor(QColor(0, 0, 0));
}

void FlatView::setNuclei(vector<Nucleus> nucs){
    nuclei = nucs;
    update();
}

void FlatView::setClusterDrops(vector<Cluster>& clusters){
    cout << "Setting the dropClusters in FlatView,, " << endl;
    dropClusters = clusters;
    update();
}

void FlatView::paintEvent(QPaintEvent* e){
    // hmm, just draw it straight let's not bother with trying to control flickering or anything
    // like that..
    QPainter p(this);
    
    vector<Nucleus>::iterator nit;
    p.setPen(QPen(QColor(255, 255, 255), 1));
    for(nit = nuclei.begin(); nit != nuclei.end(); nit++){
	// and now we draw the nucleus perimeter..
	vector<twoDPoint>::iterator tit;
	int mx, my;
	mx = my = 0;
	for(uint i=0; i < (*nit).perimeter.size(); i++){
//	for(tit = (*nit).perimeter.begin(); tit != (*nit).perimeter.end(); tit++){
	    int y = imageHeight - (*nit).perimeter[i].y;
	    int x = (*nit).perimeter[i].x;
	    mx += x;
	    my += y;
	    int r = 1;
	    if(i < (*nit).inversions.size()){
		r = int(20.0 * ((*nit).inversions[i]));
//		p.setPen(QPen(QColor(0, 0, 0), 3 * int((*nit).inversions[i])));
		//cout << "pen : " << (*nit).inversions[i] << " --> "  << int((*nit).inversions[i]) << endl;
	    }
	    p.drawPoint(x, y);
//	    r = r < 1 ? 1 : r;
	    if(r > 1){
		p.drawEllipse(x-r/2, y-r/2, r, r);
	    }
//	    p.drawLine(x, y, x, y);
	}
	QString label;
	int lwidth = 200;
	int lheight = 50;
	label.setNum((*nit).totalSignal);
	mx = mx / (*nit).perimeter.size();
	my = my / (*nit).perimeter.size();
//	p.drawText(mx-lwidth/2, my-lheight/2, lwidth, lheight, Qt::AlignCenter, label);
    }
    // and that should be it I think.. 
    for(vector<Cluster>::iterator it=dropClusters.begin(); it != dropClusters.end(); it++){
	//
	p.setPen(QPen((*it).color, 1));
	//int radius = 10;   // draw a circle around it as well as draw the actual position..
	for(vector<simple_drop>::iterator dit=(*it).members.begin(); dit != (*it).members.end(); dit++){
	    p.drawPoint((*dit).x, imageHeight - (*dit).y);
	}
    }
}

