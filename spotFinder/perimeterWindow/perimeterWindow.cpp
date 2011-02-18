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

#include "perimeterWindow.h"
#include "../perimeterSplitter.h"
#include <qlayout.h>
#include <qcolor.h>
//Added by qt3to4:
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <iostream>
#include "../../panels/fileSet.h"
//#include "../../deltaViewer.h"

using namespace std;

double scaleFactor = 10.0;

PerimeterWindow::PerimeterWindow(FileSet* fset, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
  //    deltaViewer = dv;
    fileSet = fset;
    sliceNo = -1;
    //    waveLength = 0;
    origin_x = origin_y = perimeter_w = perimeter_h = 0;
    currentSet = currentPerimeter = -1;

    plotterTextureSize = 1024;
    plotterBackground = new float[plotterTextureSize * plotterTextureSize * 3];
    plotter = new PerimeterPlotter(plotterTextureSize, this, "plotter");

    connect(plotter, SIGNAL(changeSet(int)), this, SLOT(changeSet(int)) );
    connect(plotter, SIGNAL(changePerimeter(int)), this, SLOT(changePerimeter(int)) );
    connect(plotter, SIGNAL(splitPerimeter(QList<QPolygon>)), this, SLOT(splitPerimeter(QList<QPolygon>)) );
    connect(plotter, SIGNAL(drawSelected()), this, SLOT(drawSelected()) );

    setSelector = new QSpinBox(0, 0, 1, this);
    perSelector = new QSpinBox(0, 0, 1, this);
    scaleSelector = new QSpinBox(1, 100, 1, this);
    scaleSelector->setValue(10);
    
    connect(setSelector, SIGNAL(valueChanged(int)), this, SLOT(newSet(int)) );
//    connect(setSelector, SIGNAL(valueChanged(int)), this, SLOT(redrawPerimeters(int)) );
    connect(perSelector, SIGNAL(valueChanged(int)), this, SLOT(redrawPerimeters(int)) );
    connect(scaleSelector, SIGNAL(valueChanged(int)), this, SLOT(setScale(int)) );
    
    

    QLabel* setText = new QLabel("Perimeter Sets", this);
    QLabel* perText = new QLabel("Perimeters", this);
    QLabel* scaleLabel = new QLabel("Scale", this);

    setLabel = new QLabel("0", this);
    perLabel = new QLabel("0", this);

    pixelCheck = new QCheckBox("Show background", this);
    connect(pixelCheck, SIGNAL(clicked()), this, SLOT(drawBackground()) );
    
    // some stuff for comparing outlines to each other..
    compareController = new CompareController();
    perimeterComparer = new ObjectComparer();
    QPushButton* compareButton = new QPushButton("Compare Perimeters", this);
    connect(compareButton, SIGNAL(clicked()), compareController, SLOT(show()));
    connect(compareController, SIGNAL(doFlatCompare(float, float)), this, SLOT(comparePerimeters(float, float)) );

    QPushButton* acceptButton = new QPushButton("Accept", this);
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(acceptPerimeters()) );
    
    // and make some resonable layout..
    QGridLayout* grid = new QGridLayout(this, 5, 6, 1, 1);
    grid->addMultiCellWidget(plotter, 0, 0, 0, 6);
    grid->addWidget(setText, 1, 3, Qt::AlignLeft);
    grid->addWidget(perText, 2, 3, Qt::AlignLeft);
    grid->addWidget(scaleLabel, 3, 3, Qt::AlignLeft);
    
    grid->addWidget(setLabel, 1, 4, Qt::AlignRight);
    grid->addWidget(perLabel, 2, 4, Qt::AlignRight);
    
    grid->addWidget(setSelector, 1, 5);
    grid->addWidget(perSelector, 2, 5);
    grid->addWidget(scaleSelector, 3, 5);

    grid->addWidget(pixelCheck, 3, 0);

    grid->addWidget(compareButton, 3, 1);
    grid->addWidget(acceptButton, 3, 2);
    /// stretch and spacing.. 
    grid->setRowStretch(0, 10);
}

void PerimeterWindow::changeSet(int delta){
    setSelector->setValue(setSelector->value() + delta);
}

void PerimeterWindow::changePerimeter(int delta){
    perSelector->setValue(perSelector->value() + delta);
}

void PerimeterWindow::setPerimeters(vector<PerimeterSet> perimeters, channel_info& cinfo, int sno){
    if(!perimeters.size()){
	cerr << "PerimeterWindow::setPerimeters received an empty set of perimeters,, ignoring.." << endl;
	return;
    }
    channelInfo = cinfo;
    //    waveLength = wl;
    sliceNo = sno;
//    cout << "PerimeterWindow setting perimeters.. " << endl;
    persets = perimeters;
    // then set up the various bits and pieces..
    setSelector->blockSignals(true);
    perSelector->blockSignals(true);
    
    // Labels
    QString numString;
    numString.setNum(persets.size());
    setLabel->setText(numString);

//    numString.setNum(persets[0].perimeters.size());
    perLabel->setText(numString);

    // spinBoxes..
    setSelector->setRange(0, persets.size());
    perSelector->setRange(0, persets.size());   // defaults to showing the set outlines.. 
    
    setSelector->setValue(0);
    perSelector->setValue(0);
    
    setSelector->blockSignals(false);
    perSelector->blockSignals(false);
//    cout << "and calling redrawPerimeters.. " << endl;
    redrawPerimeters(1);
}

vector<Perimeter> PerimeterWindow::selectedPerimeters(){
  vector<Perimeter> selected;
  for(uint i=0; i < persets.size(); ++i){
    if(persets[i].selectedPerimeters.size())
      selected.insert(selected.end(), persets[i].selectedPerimeters.begin(), persets[i].selectedPerimeters.end());
  }
  return(selected);
}

void PerimeterWindow::newSet(int sno){
    if(sno < 0 || (uint)sno > persets.size()){
	return;
    }
    int perNo;
    QString num;

    if(sno){
	perNo = persets[sno-1].perimeters.size();
	num.setNum(perNo);
    }
    if(sno == 0){
	perNo = persets.size();
	num.setNum(perNo);
    }
    perLabel->setText(num);
    perSelector->blockSignals(true);
    perSelector->setRange(0, perNo);
    perSelector->setValue(0);
    perSelector->blockSignals(false);
    
    redrawPerimeters(1);
}

void PerimeterWindow::setScale(int scale){
    double f_scale = double(scale / scaleFactor);
    plotter->setScale(f_scale, f_scale);
    plotter->update();
}

void PerimeterWindow::redrawPerimeters(int value){
    value = value; 
    // ignore the value its just there to allow me to do stuff..
    double scale = double(scaleSelector->value()) / scaleFactor;
//    cout << "setting scale to " << scale << endl;
    plotter->setScale(scale, scale);
//    cout << "scale set " << endl;

    if(!persets.size()){
	cerr << "PerimeterWindow::redrawPerimeters persets is 0" << endl;
	return;
    }
    // and then we need to set up a vector of outlineData (contains a QPointArray and a colour)
    // this we'll do depending on the settings..
    
    // follow these rules
    // 
    // 1. If setSelector is set to 0 we draw the outlines with the perSelector selecting which one.
    // 2. If perSelector is set to 0 draw all the lines from the set (or all the outlines).

    // use some sort of colour determination.. but work that out later..
    int setValue = setSelector->value();
    int perValue = perSelector->value();
    vector<outlineData> outlines;

//    cout << "redrawPerimeters setValue : " << setValue << "  perValue : " << perValue << endl;
    

    if(setValue == 0){
//	cout << "set value is 0" << endl;
	if(perValue == 0){
	    // in this case the current 
	    origin_x = origin_y = perimeter_w = perimeter_h = 0;
//	    cout << "\tper value is 0" << endl;
	    outlines.reserve(persets.size());
//	    cout << "persets size is  " << persets.size();
	    for(uint i=0; i < persets.size(); ++i){
		outlineData od(persets[i].outlinePerimeter.minX, persets[i].outlinePerimeter.maxX,
			       persets[i].outlinePerimeter.minY, persets[i].outlinePerimeter.maxY);
//		cout << "doing stuff with perset " << i << endl;
		vector<int>::iterator it;
		int index = 0;  // this is the stupid QPoinArray.. 
		for(it = persets[i].outlinePerimeter.perimeter.begin(); it != persets[i].outlinePerimeter.perimeter.end(); it++){
		    int x = (*it) % persets[i].outlinePerimeter.globalWidth - persets[i].outlinePerimeter.minX;
		    int y = (*it) / persets[i].outlinePerimeter.globalWidth - persets[i].outlinePerimeter.minY;
//		    cout << "  " << x << "," << y;
		    od.points.push_back(twoDPoint(x, y));
//		    od.points.push_back(twoDPoint(x, y));
		    ++index;
		}
//		cout << endl;
		// line colour is unfortunately white here..
		outlines.push_back(od);
	    }
	}else{
	    // simply define the outline for the first perset..
//	    cout << "\tper value is not 0 " << endl;
	    if((uint)perValue <= persets.size()){
		outlineData od(persets[perValue-1].outlinePerimeter.minX, persets[perValue-1].outlinePerimeter.maxX,
			       persets[perValue-1].outlinePerimeter.minY, persets[perValue-1].outlinePerimeter.maxY);
		origin_x = persets[perValue-1].outlinePerimeter.minX;
		origin_y = persets[perValue-1].outlinePerimeter.minY;
		perimeter_w = persets[perValue-1].outlinePerimeter.maxX - origin_x;
		perimeter_h = persets[perValue-1].outlinePerimeter.maxY - origin_y;
		vector<int>::iterator it;
		int index = 0;
//		cout << "collecting points for outline of set " << perValue << endl;
		for(it = persets[perValue-1].outlinePerimeter.perimeter.begin(); it != persets[perValue-1].outlinePerimeter.perimeter.end(); it++){
		    int x = (*it) % persets[perValue-1].outlinePerimeter.globalWidth - persets[perValue-1].outlinePerimeter.minX;		    
		    int y = (*it) / persets[perValue-1].outlinePerimeter.globalWidth - persets[perValue-1].outlinePerimeter.minY;
//		    cout << "  " << x << "," << y;
		    od.points.push_back(twoDPoint(x, y));
		    ++index;
		}
//		cout << endl;
		outlines.push_back(od);
	    }
	}
    }
    if(setValue && (uint)setValue <= persets.size()){
	setValue--;
//	cout << "looking at set " << setValue << endl;
	origin_x = persets[setValue].outlinePerimeter.minX;
	origin_y = persets[setValue].outlinePerimeter.minY;
	perimeter_w = persets[setValue].outlinePerimeter.maxX - origin_x;
	perimeter_h = persets[setValue].outlinePerimeter.maxY - origin_y;
	
	if(perValue == 0){
//	    cout << "getting outline for all perimters of set " << setValue << endl;
	    outlines.reserve(persets[setValue].perimeters.size());
	    vector<Perimeter>::iterator pit;
	    int col_i = 1;
	    for(pit = persets[setValue].perimeters.begin(); pit != persets[setValue].perimeters.end(); pit++){
		outlineData od(persets[setValue].outlinePerimeter.minX, persets[setValue].outlinePerimeter.maxX,
			       persets[setValue].outlinePerimeter.minY, persets[setValue].outlinePerimeter.maxY);
		int index = 0;
		vector<int>::iterator it;
		
		// calculate a colour .. in some reasonable manner..
		int m = 10;  // a multiplier of som sorts 
		int r = (col_i * m) % 256;
		int b = 255 - r;
		int g = (col_i * 5) % 256; 
		od.c = QColor(r, g, b);
		col_i++;
		for(it = (*pit).perimeter.begin(); it != (*pit).perimeter.end(); it++){
		    int x = (*it) % (*pit).globalWidth - persets[setValue].outlinePerimeter.minX;
		    int y = (*it) / (*pit).globalWidth - persets[setValue].outlinePerimeter.minY;
		    od.points.push_back(twoDPoint(x, y));
		    ++index;
		}
		outlines.push_back(od);
	    }
	}else{
	    if((uint)perValue <= persets[setValue].perimeters.size()){
		perValue--;
		outlineData od(persets[setValue].outlinePerimeter.minX, persets[setValue].outlinePerimeter.maxX,
			       persets[setValue].outlinePerimeter.minY, persets[setValue].outlinePerimeter.maxY);
//		cout << "getting outline for set " << setValue << "  perimter " << perValue << endl;
		Perimeter& perimeter = persets[setValue].perimeters[perValue];
		for(uint i=0; i < perimeter.perimeter.size(); ++i){
		    int x = perimeter.perimeter[i] % perimeter.globalWidth - persets[setValue].outlinePerimeter.minX;
		    int y = perimeter.perimeter[i] / perimeter.globalWidth - persets[setValue].outlinePerimeter.minY;
		    od.points.push_back(twoDPoint(x, y));
		}
		outlines.push_back(od);
	    }
	}
    }
    
    if(setSelector->value() and perSelector->value()){
	const PerimeterParameters& pers = persets[setValue].perimeters[perValue].parameters;
	cout << "Perimeter Parameters : " << endl
	     << "\tlength : " << pers.length << endl
	     << "\tarea   : " << pers.area << endl
	     << "\tsignal sum  : " << pers.signalSum << "\tmean : " << pers.signalSum/(float)pers.area << endl
	     << "\t 10, 50, 90 : " << pers.signal_10 << "\t" << pers.signal_50 << "\t" << pers.signal_90 << endl
	     << "\tcenter dist : " << pers.mean_cd << "\t" << pers.std_cd << endl
	     << "\tcenter 10, 50, 90 : " << pers.cd_10 << "\t" << pers.cd_50 << "\t" << pers.cd_90 << endl;
    }

    // and at this point lets just call the appropriate..
//    cout << "calling plotter set lines" << endl;
    plotter->setLines(outlines);
//    cout << "calling plotter to update " << endl;

    if(pixelCheck->isChecked() && currentSet != setValue){
	currentSet = setValue;
	currentPerimeter = perValue;
	drawBackground();
	return; // we don't want to draw 2 times.. drawbackground doesn't update
    }
    // but regardless we have to make sure that currentPerimeter is updated
    currentPerimeter = perValue;
    plotter->update();
//    cout << "plotter update returned" << endl;
}
	
void PerimeterWindow::drawBackground(){
    // check if pixelCheck is selected and if this is the case then obtain the appropriate map from the thingy and send it to the thingy ..
    cout << "drawBackground was called .." << endl;
    if(!pixelCheck->isChecked()){
	cout << "\tbut pixelCheck isn't checked" << endl;
	return;
    }
    // check width & height..
    if(perimeter_w <= 0 || perimeter_h <= 0){
	cout << "but we don't have much of a perimeter.. " << endl;
	return;   // this isn't actually an error, as it can be the case when things are doing stuff..
    }
    // and now we can make a pixmap of values ..
    // (note that these are not scaled in any way,, we can think of other ways of doing that..)

    memset((void*)plotterBackground, 0, plotterTextureSize * plotterTextureSize * 3 * sizeof(float));

    cout << "drawBackground trying to get background for : " << origin_x << "," << origin_y << "\t:" << perimeter_w << "," << perimeter_h << endl;
//    if(deltaViewer->readToRGB(plotterBackground, origin_x, origin_y, (uint)plotterTextureSize, (uint)plotterTextureSize, (uint)sliceNo)){
    vector<channel_info> cinfo;
    cinfo.push_back(channelInfo); // 
    if(!fileSet->readToRGB(plotterBackground, origin_x, origin_y, (uint)perimeter_w, (uint)perimeter_h, (uint)sliceNo, cinfo))
      cerr << "PerimeterWindow::drawBackground unable to read in background data from fileSet" << endl;
    
//     if(deltaViewer->readToRGB(plotterBackground, origin_x, origin_y, (uint)perimeter_w, (uint)perimeter_h, (uint)sliceNo)){
// 	cout << "drawBackground successfully read in the data .." << endl;
//     }else{
// 	cout << "drawBackground had some problem in getting the data" << endl;
// //	delete pixels;
// 	return;
//     }
    
    // if requested show the mask as a red background 
    int perValue = setSelector->value();
    perValue--;
//     cout << "Trying to draw mask " << perValue << endl;

//     if(perValue >= 0 && perValue < persets.size()){
// 	for(int y=0; y < perimeter_h; ++y){
// //	    cout << "x";
// 	    int gy = origin_y + y;
// 	    if(gy >= persets[perValue].mask_oy && gy < persets[perValue].mask_oy + persets[perValue].mask_h){
// //		cout << "y";
// 		for(int x=0; x < perimeter_w; ++x){
// 		    int gx = x + origin_x;
// //		    cout << ".";
// 		    if(gx >= persets[perValue].mask_ox && gx < persets[perValue].mask_ox + persets[perValue].mask_w){
// //			cout << "-";
// 			// then work out the position in the mask and if it's equal to outvalue then set 
// 			// the colour in plotterBackground to something..
// 			if(persets[perValue].mask[(gy - persets[perValue].mask_oy) * persets[perValue].mask_w + gx - persets[perValue].mask_ox]
// 			   == persets[perValue].outvalue){
// 			    plotterBackground[3 * (y * perimeter_w + x)] = 0.8;  // bright red..
// //			    cout << "|";
// 			}
// 		    }
// 		}
// 	    }
// //	    cout << endl;
// 	}
//     }
	

//    plotter->setBackground(plotterBackground, plotterTextureSize, plotterTextureSize);
    plotter->setBackground(plotterBackground, perimeter_w, perimeter_h);

//    delete pixels;
    
    plotter->update();
}

void PerimeterWindow::comparePerimeters(float sigma, float order){
    // Since every perimeter is actually compared independently we'll need to prepare the parameters from that perimeter
    // seperately. Since we are using complicated stuff we'll need to count the perimeter number first..
    cout << "comparePerimeters in PerimeterWindow" << endl;
    int perNo = 0;
    for(uint i=0; i < persets.size(); i++){
	perNo += persets[i].perimeters.size();
    }

    if(!perNo){
	cerr << "no perimeters specified.. " << endl;
	return;
    }

    int parNo = 13;
    float* data = new float[perNo * parNo];
    memset((void*)data, 0, sizeof(float) * perNo * parNo);
    
    int* objectIds = new int[perNo];
    // and then simply..
    uint p = 0;
    for(uint i=0; i < persets.size(); i++){
	for(uint j=0; j < persets[i].perimeters.size(); ++j){
	    const PerimeterParameters& pr = persets[i].perimeters[j].parameters;
	    float* dpt = data + p * parNo;
	    dpt[0] = (float)pr.length;
	    dpt[1] = (float)pr.area;
	    dpt[2] = float(pr.length)/float(pr.area);
	    dpt[3] = pr.signalSum;
	    dpt[4] = pr.signalSum/float(pr.area);
	    dpt[5] = pr.signal_10;
	    dpt[6] = pr.signal_50;
	    dpt[7] = pr.signal_90;
	    dpt[8] = pr.mean_cd;
	    dpt[9] = pr.std_cd;
	    dpt[10] = pr.cd_10;
	    dpt[11] = pr.cd_50;
	    dpt[12] = pr.cd_90;
	    ++p;
	}
    }
    // and then we can just start the objectComparer..
    cout << "Setting the data and doing the comparison for the perimeters (total of " << perNo << ")" << endl;
    perimeterComparer->setData(data, perNo, parNo, sigma, order);
    compareController->newDistances(perimeterComparer->distances());
    cout << "comparison done" << endl;
}

vector<int> PerimeterWindow::boundaryFromPolygon(QPolygon poly, int xo, int yo, int gw){
    // not sure of the best way of doing this, but lets have a go..
    vector<int> points;
    if(poly.size() < 2){
	cerr << "PerimeterWindow::boundaryFromPolygon, points is too small (less than 2) " << poly.size() << endl;
	return(points);
    }
    for(uint i=1; i < poly.size(); i++){   
	// Fill in the points from i-1 to i
	QPoint p1 = poly[i-1];
	QPoint p2 = poly[i];
//	points.push_back((p1.y() + yo) * gw + p1.x() + xo);
	// then work out how to get the other points..
	int dx = p2.x() - p1.x();
	int dy = p2.y() - p1.y();
	int stepNo = abs(dx) > abs(dy) ? abs(dx) : abs(dy);  
        // in the funny world of pixels a diagonal is only as long as the longer side..
	for(int i=0; i < stepNo; i++){
	    int lx = p1.x() + (i * dx)/stepNo;
	    int ly = p1.y() + (i * dy)/stepNo;
	    // then convert and push back..
	    points.push_back((ly + yo) * gw + lx + xo);
	}
    }
    // but at this point we have not added the last point so we need to do that..
    QPoint p = poly.back();
    points.push_back( (p.y() + yo) * gw + p.x() + xo);
    return(points);
}

void PerimeterWindow::splitPerimeter(QList<QPolygon> polys){
    cout << "PerimeterWindow::splitPerimeter" << endl;
    // first go throught the polygons and get a vector of useful information.
    // in order to do this, I need to first work out what is the current perimter set and the
    // current perimeter of that set.
     
    // make sure that the set and perimeter values are reasonable..
    // unfortunately currentSet and currentPerimeter may not be accurate so..
    int setNo = setSelector->value() - 1;
    int perNo = perSelector->value() - 1;
    if(setNo < 0){
	cerr << "split Perimter, currentSet (" << setNo << ") or perimeter (" << perNo << ") has bad value" << endl;
	return;
    }
 
    Perimeter& perimeter = persets[setNo].perimeters[0];
    if(perNo >= 0)
      perimeter = persets[setNo].perimeters[perNo];

//     if(perNo < 0){
// 	perimeter = persets[setNo].perimeters[0];
// //	perimeter = &persets[setNo].outlinePerimeter;
//     }else{
// 	perimeter = persets[setNo].perimeters[perNo];
//     }

   cout << "\tcurrentSet       " << setNo << endl
	<< "\tcurrentPerimeter " << perNo << endl;
   // I should probably do a bounds checking on the below.. but ..
    int gw = (int)perimeter.globalWidth;
//    int pw = perimeter->maxX - perimeter->minX;

    // then just use origin_x and origin_y to work out how to do the whole thing..
    vector<vector<int> > boundaries;
    for(uint i=0; i < polys.size(); ++i){
	boundaries.push_back(boundaryFromPolygon(polys[i], origin_x, origin_y, gw));
    }
    // and let's see if that works ok.. 
    // and lets get some new perimeters from the currentperimeter..
    cout << "Calling splitlines on perimeter size of boundaries is " << boundaries.size() << endl
	 << "                               and origin is          " << origin_x << origin_y << endl;
    
    PerimeterSplitter splitter;
    splitter.splitPerimeter(perimeter, boundaries);
    vector<vector<int> > newPer = splitter.newPerimetersV();

    //    vector<vector<int> > newPer = perimeter.splitPerimeters(boundaries);
    // cout << "obtained a total of " << newPer.size() << " new perimeters from perimeter " << endl;

    // let's make a vector of outline data to allow us to draw the rest..
    vector<outlineData> outlines;
    for(uint i=0; i < newPer.size(); ++i){
	outlineData od; // it seems we never actually use the minX/maxX so no need to really do anything..
	for(uint j=0; j < newPer[i].size(); j++){
	    od.points.push_back(twoDPoint(newPer[i][j] % gw - origin_x, newPer[i][j] / gw - origin_y));
	}
	outlines.push_back(od);
    }
    if(outlines.size()){
	persets[setNo].setSelection(newPer);
    }
    // and then we try setting the data for the plotter .. 
    plotter->setLines(outlines);
    plotter->update();
}

void PerimeterWindow::drawSelected(){
    int setNo = setSelector->value() - 1;
    if(setNo < 0){
	return;
    }
//    cout << "perimeterWindow::drawSelected number of lines " << persets[setNo].selectedPerimeters.size() << endl;
    int gw = persets[setNo].outlinePerimeter.globalWidth;
    int pw = persets[setNo].outlinePerimeter.maxX - persets[setNo].outlinePerimeter.minX;
    vector<outlineData> outlines;
    for(vector<Perimeter>::iterator it=persets[setNo].selectedPerimeters.begin(); it != persets[setNo].selectedPerimeters.end(); it++){
	outlineData od;
//	cout << "-----" << endl;
	for(vector<int>::iterator vit=(*it).perimeter.begin(); vit != (*it).perimeter.end(); vit++){
	    od.points.push_back(twoDPoint((*vit) % gw - origin_x, (*vit) / gw - origin_y));
//	    cout << (*vit) % gw - origin_x << "," << (*vit) / gw - origin_y << "  ";
	}
	outlines.push_back(od);
//	cout << "xxxxxxxx" << endl;
    }
    plotter->setLines(outlines);
    plotter->update();
}

void PerimeterWindow::acceptPerimeters(){
  // This signal should be changed to use the wave index, but I need to 
  // change it to something else.. 
  emit  perimetersFinalised(persets, channelInfo.finfo.emission, sliceNo);
  //    emit perimetersFinalised(persets, waveLength, sliceNo);
}
