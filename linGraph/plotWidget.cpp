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

#include "plotWidget.h"
#include <qpixmap.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <iostream>

using namespace std;

PlotWidget::PlotWidget(QWidget* parent, const char* name)
    : QWidget(parent,name)
{
    backgroundColor = QColor(0, 0, 0);   // yeah let's make it black when we draw.
    setBackgroundMode(Qt::NoBackground);
    rangeSelector = new Slider(QRect(0, 0, 0, 0), 0, 0);  // just some numbers to make it happy
    minY = maxY = 0;
    maxX = 0;
    sliderState = 0;
    lMargin = 30;
    rMargin = 30;
}

PlotWidget::~PlotWidget(){
    delete rangeSelector;
}

void PlotWidget::setValues(vector<vector<float> > v){
    vector<int> temp;
    setValues(v, temp, lMargin, rMargin, 0, 0);
}

void PlotWidget::setValues(vector<vector<float> > v, vector<int> m, int LMargin, int RMargin, float MinY, float MaxY){
    if(!v.size()){
	cerr << "v size is 0 " << endl;
	return;
    }
    values = v;
    marks = m;
    lMargin = LMargin;
    rMargin = RMargin;
//     for(uint i=0; i < values[0].size(); i++){
// 	cout << values[0][i] << ", ";
//     }
//     cout << endl;
    // and tell the sliding range selector something about this..
    // rangeSelector->setRange(0, v.size());
    // if minY and max Y are something then set to this, otherwise find out the min and max y..
    if(MaxY - MinY == 0){
	minY = maxY = values[0][0];
	for(uint i=0; i < values.size(); i++){
	    for(uint j=0; j < values[i].size(); j++){
		if(minY > values[i][j]){ minY = values[i][j]; }
		if(maxY < values[i][j]){ maxY = values[i][j]; }
	    }
	}
    }else{
	minY = MinY;
	maxY = MaxY;
    }
    // this way the user can specify a range or leave the program to autorange it.. 
    // let's specify the range Selector..
    maxX = values[0].size();
    for(uint i=0; i < values.size(); i++){
	if(maxX < values[i].size()){
	    maxX = values[i].size();
	}
    }
    rangeSelector->setRange(0, maxX);
//    rangeSelector->setRange(0, values[0].size());
    
    // and let's set the colours if necessary..
    for(uint i=colors.size(); i < values.size(); i++){
	colors.push_back(QColor(255, 255, 255));
    }
    // and let's make sure to have reasonable limits..
    for(uint i=minPeakValues.size(); i < values.size(); i++){
	minPeakValues.push_back(minY);
    }
    for(uint i=maxEdgeValues.size(); i < values.size(); i++){
	maxEdgeValues.push_back(minY);
    }
    for(uint i=scales.size(); i < values.size(); i++){
	scales.push_back(1.0);
    }
    

    // and let's redraw..
    repaint();
}

void PlotWidget::paintEvent(QPaintEvent* pe){
    // set the background as black..
    QPixmap pix(size());
    pix.fill(backgroundColor);
    // set the rangeSelector.. 
    // set it at a set position from the bottom of the widget..
    int vm = 20;
    int hm = 40;
    int selectorHeight = 11;
    rangeSelector->setPosition(QRect(hm, height() - (vm + selectorHeight), width() - 2 * hm, selectorHeight));

    int graphBottom = height() - (2 * vm + selectorHeight + 10);   // ugly to have the 10 there, but.. I'm sleepy.
    
    // at which point let's start some drawing..
    QPainter p(&pix);
    rangeSelector->draw(&p);
    
    // first draw the limit lines..
    float h = float(graphBottom - vm);
    float yrange = maxY - minY;
    for(uint i=0; i < values.size(); i++){
	if(colors[i].blue() + colors[i].red() + colors[i].green()){
	    p.setPen(QPen(colors[i], 1));
	    int y = graphBottom - int(scales[i] * h * (minPeakValues[i] - minY)/yrange );
	    p.drawLine(hm, y, width() - hm, y);
	    y = graphBottom - int(scales[i] * h * (maxEdgeValues[i] - minY)/yrange );
	    p.drawLine(hm, y, width() - hm, y);
	}
    }
    
    // ticks and other things I think that I can do with out..
    int b = rangeSelector->begin();
    int e = rangeSelector->end();
    // make sure that they are ok..
    //cout << "Before : b " << b << "\te " << e << endl;
    int w = width() - 2*hm;
    int range = e - b;
    // and if we have them let's draw peak lines in the appropriate colours..
    for(map<int, vector<int> >::iterator it=peaks.begin(); it != peaks.end(); it++){
	if((*it).first >= colors.size()){
	    cerr << "peak identifier too large for thingy .. " << endl;
	    continue;
	}
	p.setPen(QPen(colors[(*it).first], 1));
	for(vector<int>::iterator vit=(*it).second.begin(); vit != (*it).second.end(); vit++){
	    if((*vit) > b && (*vit) < e){
		int xpos = hm + (((*vit) - b) * w)/(range -1);
		p.drawLine(xpos, graphBottom, xpos, vm);
	    }
	}
    }


    p.setPen(QPen(QColor(255, 255, 255), 1));   // do I really want a white pen.. ?
    p.drawLine(hm, vm, hm, graphBottom);
    p.drawLine(hm, graphBottom, width()-hm, graphBottom);
    
    if(values.size() && maxX){
//    if(values.size() && values[0].size()){
	if(e > maxX || e < 0){ e = maxX; }
	if(b < 0 || b > maxX){ b = 0; }
//	if(e > values[0].size() || e < 0){ e = values[0].size(); }
//	if(b < 0 || b > values[0].size()){ b = 0; }
	//cout << "After  : b " << b << "\te " << e << endl;
	range = e - b;
	for(uint g=0; g < values.size(); g++){
	    //cout << "drawing first channel " << endl;
	    if(colors[g].blue() + colors[g].red() + colors[g].green()){
		p.setPen(QPen(colors[g], 1));
		for(int i=0; i < range-1; i++){
		    // draw a line from b+i  --> b+i+1;
		    if(i + b < values[g].size()){
			int x1 = hm + (i * w) / (range - 1);
			int x2 = hm + ((i+1) * w) / (range - 1);
			int y1 = graphBottom - int(scales[g] * h * (values[g][i+b] - minY)/yrange );
			int y2 = graphBottom - int(scales[g] * h * (values[g][i+b+1] - minY)/yrange );
			//cout << "drawing : " << x1 << "\t" << x2 << "\t" << values[g][i+b] << "\t" << values[g][i+b+1] << "\t" << y1 << "\t" << y2 << endl;
			p.drawLine(x1, y1, x2, y2);
		    }
		}
	    }
	}
    }else{
	//cerr << "either values size isn't or values[0] size isn't " << values.size() << endl;
    }
    p.setPen(QPen(QColor(255, 255, 255)));
    for(uint i=0; i < marks.size(); i++){
	if(marks[i] > b && marks[i] < e){
	    int x = hm + ((marks[i]-b) * w) / (range-1);
	    p.drawLine(x, graphBottom-5, x, vm);
	}
    }
    // and I somehow expect that that is it. now for bitblt..
    bitBlt(this, 0, 0, &pix, 0, 0);
}

void PlotWidget::mousePressEvent(QMouseEvent* e){
    lastX = e->x();
    lastY = e->y();
    if(rangeSelector->outerContains(e->x(), e->y())){
	switch(e->button()){
	    case Qt::LeftButton :
		sliderState = 1;
		break;
	    case Qt::RightButton :
		sliderState = 2;
		break;
	    case Qt::MidButton :
		sliderState = 3;
		break;
	    default :
		cerr << "unknown button state " << e->button()  << endl;
	}
    }
    
}
	
void PlotWidget::mouseMoveEvent(QMouseEvent* e){
    int dx = e->x() - lastX;
    int dy = e->y() - lastY;
    lastX = e->x();
    lastY = e->y();
    int dl = 0;
    int dh = 0;
    switch(sliderState){
	case 0 :
	    break;
	case 1:
	    dl = dx;
	    break;
	case 2:
	    dh = dx;
	    break;
	case 3:
	    dl = dh = dx;
	    break;
	default :
	    cerr << "unknown sliderState " << sliderState << endl;
    }
    if(dl || dh){  // need to set the new parameters and to redraw the graph..
        rangeSelector->adjustLimits(dl, dh);
	update();
    }
}

void PlotWidget::mouseReleaseEvent(QMouseEvent* e){
    // do nothing for now..
}

void PlotWidget::setColors(vector<QColor> Colors){
    colors = Colors;
//     if(colors.size() <= Colors.size()){
// 	colors = Colors;
//     }else{
// 	for(uint i=0; i < Colors.size(); i++){
// 	    colors[i] = Colors[i];
// 	}
//     }
    update();
}

void PlotWidget::wheelEvent(QWheelEvent* e){
    emit(incrementImage(e->delta()/120));
}

void PlotWidget::setMinPeakValue(unsigned int id, float mpv){
    cout << "setting minpeak value of " << id << " to  " << mpv << endl;
    if(id < minPeakValues.size()){
	minPeakValues[id] = mpv;
	update();
    }
}

void PlotWidget::setMaxEdgeValue(unsigned int id, float mev){
    if(id < maxEdgeValues.size()){
	maxEdgeValues[id] = mev;
	update();
    }
}

void PlotWidget::setScale(unsigned int id, float s){
    if(id < scales.size()){
	scales[id] = s;
	update();
    }
}

void PlotWidget::setPeaks(map<int, vector<int> > Peaks){
    peaks = Peaks;
    //update();
}

void PlotWidget::clearPeaks(){
    peaks.erase(peaks.begin(), peaks.end());
}
