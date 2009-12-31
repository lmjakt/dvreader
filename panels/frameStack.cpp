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

#include "frameStack.h"
#include "../image/imageFunc.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdio.h>

using namespace std;

FrameStack::FrameStack(int* waveLengths, int waveNo, ifstream* inStream, float maxLevel){
    maxIntensity = maxLevel;
    wave_lengths = waveLengths;
    wave_no = waveNo;
    fwaves.resize(wave_no);
    focalPlanes.resize(wave_no);
    frameInformation = 0;
    for(int i=0; i < wave_no; i++){
	fwaves[i] = float(wave_lengths[i]);
	focalPlanes[i] = 0;
    }
    sort(fwaves.begin(), fwaves.end());
    
    
    in = inStream;


    leftNeighbour = topNeighbour = rightNeighbour = bottomNeighbour = 0;
    leftOverlap = topOverlap = rightOverlap = bottomOverlap = 0;
    x = y = 0;
    width = height = 0;
    z_begin = z_end =  0;
    pWidth = pHeight = 0;
    pixelX = pixelY = -1;    // which should be an undefined position.. 
    finalised = false;
    positionAdjusted = 0;
    neighboursAdjusted = false;
    projection = 0;
}

FrameStack::~FrameStack(){
    while(sectionMap.size()){
	FrameSet* fs = (*sectionMap.begin()).second;
	delete fs;
	sectionMap.erase(sectionMap.begin());
    }
    if(projection){
	for(uint i=0; i < fwaves.size(); ++i){
	    if(projection[i]){
		delete projection[i];
	    }
	}
	delete projection;
    }

    delete in;
}

bool FrameStack::addFrame(const char* fname, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
				 short numInt, short numFloat, unsigned short byteSize,
				 bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz, Frame*& frame)
{
    ifstream* in = new ifstream(fname);
    if(!(*in)){
	cerr << "FrameStack unable to open " << fname << " for reading" << endl;
	delete in;
	return(false);   // but no error for the caller.. hmm...
    }
   
    frame = new Frame(in, framePos, readPos, extHeadSize, numInt, numFloat, byteSize, real, bigEnd, width, height, dx, dy, dz);
    
    return(addFrame(frame));
}

void FrameStack::setBackgrounds(std::map<fluorInfo, Background*> backgrounds){
  for(uint i=0; i < sections.size(); ++i)
    sections[i]->setBackgrounds(backgrounds, i);
}

bool FrameStack::addFrame(Frame* frame){
  //cout << "FrameStack::addFrame excite : " << frame->excitation() << endl;
    if(!frame->ok()){
	cerr << "FrameStack::addFrame frame reports itself as being not ok. bummer" << endl;
	return(false);
    }
    if(!width){
	// just insert the frame into this one and set the appropriate parameters..
	x = frame->xPos();
	y = frame->yPos();
	z_begin = z_end = frame->zPos();
	width = frame->sampleWidth();
	height = frame->sampleHeight();
	pWidth = frame->p_width();
	pHeight = frame->p_height();
	
	// then make a new frameSet and add the frame to that..
	FrameSet* fset = new FrameSet(wave_lengths, wave_no);
	if(!fset->addFrame(frame)){
	    cerr << "FrameStack : unable to add frame to newly created frameset" << endl;
	    return(false);
	}
	sectionMap.insert(make_pair(z_begin, fset));
	return(true);
    }
    // if we have width then we have to determine whether or not this frame belongs to this frameStack..
    // it's x and y positions should be the same.. if not then we have to let the owner of this object take
    // care of it..
    if(frame->xPos() != x || frame->yPos() != y){
	cerr << "Frame appears to not belong to this framestack: " << x << " != " << frame->xPos() << " || " << y << " != " << frame->yPos() << endl;
	return(false);
    }
    // then we try to find a frameset with the same z - position..
    map<float, FrameSet*>::iterator it = sectionMap.find(frame->zPos());  // this may fail if the positions are not precise..
    if(it == sectionMap.end()){
	//cerr << "SectionStack -- couldn't find a section at z position : " << frame->zPos() << endl;
	// in this case we need to make a new frameSet and add the section..
      FrameSet* fset = new FrameSet(wave_lengths, wave_no);
      //cout << "made new frameset for excite " << frame->excitation() <<  "  and will add " << endl;
	if(!fset->addFrame(frame)){
	    cerr << "FrameStack : unable to add frame to newly created frameset (2)" << endl;
	    return(false);
	}
	sectionMap.insert(make_pair(frame->zPos(), fset));
	return(true);
    }
    // and otherwise we just add the frame stack to the frame as we should..
    if(!(*it).second->addFrame(frame)){
	cerr << "FrameStack add frame : unable to add frame to existing frame set at position " << frame->zPos() << endl;
	// if this should happen, I will have a problem,, as I don't have any good option.
	// might be better to return an error code rather than a simple boolean, but for now leave it as it is
	return(false);
    }
    return(true);
}

void FrameStack::finalise(float maxLevel, FrameInfo* frameData){
//void FrameStack::finalise(float maxLevel, float** projectionData){
    sections.resize(0);
    for(map<float, FrameSet*>::iterator it=sectionMap.begin(); it != sectionMap.end(); it++){
	sections.push_back((*it).second);
    }
    finalised = true;   // but it would be better to actually check if we have all the slices.. 
    if(frameData){   // if it has been read in from a file...
	// if we already have a frameInformation then we should delete that .. 
	
	/// BUT, we don't know who made the frameInformation.. - so we don't actually know..
	/// this shouldn't really happen, but it is a potential memory leak. bummer. bad design. 

	frameInformation = frameData;
	pixelX = frameInformation->xp;
	pixelY = frameInformation->yp;
	
	leftBorder = pixelX > leftBorder ? pixelX : leftBorder;
	rightBorder = pixelX + int(pWidth) < rightBorder ? pixelX + pWidth : rightBorder;
	topBorder = pixelY + int(pHeight) < topBorder ? pixelY + pHeight : topBorder;
	bottomBorder = pixelY > bottomBorder ? pixelY : bottomBorder;

	//cout << "\t\tpixelX : " << pixelX << "\tpixelY : " << pixelY << endl;
	
	cout << "FrameStack::finalise projection data has been read in from a file" << endl;
	projection = frameInformation->projection;   // though we may not need to know the projection .. 
	/// currently many pieces of code refer to projection directly, we should try to remove this in the future. 
	return;
    }
    // first initialise the projection
    projection = new float*[fwaves.size()];
    // resize the contrast data ..
    contrasts.resize(fwaves.size());
    for(uint i=0; i < fwaves.size(); i++){
	projection[i] = make_mip_projection(i, maxLevel, contrasts[i]);
//	projection[i] = make_mip_projection(fwaves[i], maxLevel, contrasts[i]);
    }
    frameInformation = new FrameInfo(fwaves.size(), projection, x, y, pixelX, pixelY, pWidth, pHeight);

}

void FrameStack::printFrames(){
    map<float, FrameSet*>::iterator it;
    int counter = 0;
    for(it = sectionMap.begin(); it != sectionMap.end(); it++){
	cout << "\t" << counter++ << "\t" << (*it).second->z_pos() << "\t" << (*it).second->x_pos() << ", " << (*it).second->y_pos() 
	     << "  size : " << (*it).second->width() << ", " << (*it).second->height() << "\t" << (*it).second->num_colors() << endl;
    }
}

bool FrameStack::setNeighbour(FrameStack* neibour, int pos, bool recip){
    bool olaps = false;
//    cout << "FrameStack::setNeighbour pos : " << pos << "\trecip : " << recip << endl;
    switch(pos){
	case 0:
	    // left position
	    leftNeighbour = neibour;
	    leftOverlap = neibour->x_pos() + neibour->imageWidth() - x;
	    olaps = leftOverlap > 0;
//	    cout << "left overlap : " << leftOverlap << endl;
	    if(recip){
		neibour->setNeighbour(this, 2, false);
		leftBorder = neibour->setBorder((left() + neibour->right())/2, RIGHT);    // set the left neighbour's right border .. 
	    }
	    break;
	case 1:
	    // top position
	    topNeighbour = neibour;
	    topOverlap = y + height - neibour->y_pos();
	    olaps = topOverlap > 0;
//	    cout << "top overlap " << topOverlap << endl;
	    if(recip){
		neibour->setNeighbour(this, 3, false);
		topBorder = neibour->setBorder((top() + neibour->bottom())/2, BOTTOM);
	    }
	    break;
	case 2:
	    // right position
	    rightNeighbour = neibour;
	    rightOverlap = x + width - neibour->x_pos();
	    olaps = rightOverlap > 0;
//	    cout << "rightOverlap " << rightOverlap << endl;
	    if(recip){
		neibour->setNeighbour(this, 0, false);
		rightBorder = neibour->setBorder((right() + neibour->left())/2, LEFT);
	    }
	    break;
	case 3: 
	    // bottom position
	    bottomNeighbour = neibour;
	    bottomOverlap = neibour->y_pos() + neibour->imageHeight() - y;
	    olaps = bottomOverlap > 0;
//	    cout << "bottomOverlap " << bottomOverlap << endl;
	    if(recip){
		neibour->setNeighbour(this, 1, false);
		bottomBorder = neibour->setBorder((bottom() + neibour->top())/2, TOP);
	    }
	    break;
	default :
	    cerr << "FrameStack::setNeighbour unknown neighbour type : " << pos << endl;
    }
    return(olaps);
}

int FrameStack::setBorder(int pos, POSITION n){
    int p = -1;  // 
    switch(n){
	case LEFT :
	    // do stuff..
	    leftBorder = pos;
	    p = pos;// - 1;
	    break;
	case TOP :
	    // do stuff..
	    topBorder = pos;
	    p = pos;// + 1;
	    break;
	case RIGHT :
	    // do stuff.
	    rightBorder = pos;
	    p = pos;// + 1;
	    break;
	case BOTTOM :
	    // do stuff 
	    bottomBorder = pos;
	    p = pos;// - 1;
	    break;
	default :
	    cerr << "FrameStack::setBorder unknonw position given " << n << endl;
    }
    //cout << "Border set to " << p << endl;
    return(p);
}

vector<overlap_data*> FrameStack::adjustNeighbourPositions(unsigned int secNo, unsigned int rolloff, int instep, int window, int px, int py){
    ///////////// IMPORTANT NOTE //////////
    ////////////  THE WAVELENGTH ISN'T ACTUALLY USED. ALL THE WAVELENGTHS ARE SCANNED IN THE findNeighbourOffset function that gets called
    ///////////   from here. We should remove this to avoid any confusion... 


//    cout << "FrameStack::adjustNeighbourPositions wavelegnth " << wavelength << endl;
    float minCorr = 0.5;
    vector<overlap_data*> overlaps;

    // first work out the waveIndex as we want to use a different function//
//     int waveIndex = 0;
//     for(uint i=0; i < fwaves.size(); i++){
// 	if(fwaves[i] == wavelength){
// 	    waveIndex = i;
// 	}
//     }
//     cout << "FrameStack::adjustNeighbourPositions waveIndex is : " << waveIndex << endl;
    
    if(neighboursAdjusted){  // don't do more than once.. 
	return(overlaps);
    }
    neighboursAdjusted = true;
//    overlap_data* olapData = new overlap_data();
//    if(secNo >= sections.size()){
//	cerr << "FrameStack::adjustNeighbourPositions secNo is larger than sections.size() : " << secNo << endl;
//	return overlaps;
//    }
//    if(!sections[secNo]->hasWavelength(wavelength)){
//	cerr << "FrameStack::adjustNeighbourPositions : unknown wavelength " << wavelength << endl;
//	return overlaps;
//    }
    
//    if(!rightNeighbour && !topNeighbour){
//	cerr << "FrameStack::adjustNeighbourPositions: No appropriate neigbhours defined returning to where I came from" << endl;
//	return overlaps;
//    }
    if(window >= (instep-1) || window < 0){    // instep is given as the number of lines inside of the rolloff.. 
	window = instep -1 ;
    }
    if(instep < 0){
	cerr << "FrameStack::adjustNeighbourPositions instep is less than 0 : " << instep << endl;
	return overlaps;
    }

    ImageFunc iFunc;
    int margin = 50;

    if(rightNeighbour && !rightNeighbour->adjustedNeighbours()){   // then we may change the position of the right neighbour.. 
//    if(rightNeighbour && !(rightNeighbour->isPositionAdjusted())){   // then we may change the position of the right neighbour.. 
//	cout << "lx : " << lx << "  neighbour xpos : " << rightNeighbour->x_pos() << "\t  dx " << dx << endl;
	int gx = (rightNeighbour->left() + right()) / 2;
//	int gx = pixelX + pWidth - rolloff - instep - window;

	int nlix = gx - rightNeighbour->left();
	int tlix = gx - left();       // the starting positions for this and the neighbour..

	int gy1 = pixelY + rolloff + margin;
	int gy2 = pixelY + pHeight - margin - rolloff;
	int ny1 = gy1 - rightNeighbour->bottom();
	int y1 = gy1 - pixelY;

	if( (window + tlix) >= (pWidth - rolloff)){
	    cout << "adjustNeighbourPositions : window changing from " << window;
	    window = ((pWidth - rolloff) - tlix);
	    cout << "\tto : " << window << endl;
	}
	if(window <= 0){
	    cerr << "adjustNeighbourPositions window is less than 0 : " << window << endl;
	    return(overlaps);
	}

	// let's check the function to see what happens when we use an imageFuncObject instead..
	int areaH = gy2 - gy1;
	int areaW = window * 2 + 1;

	float* neighborArea = new float[areaH * areaW];
	float* thisArea = new float[areaH * areaW];

	nlix -= window;
	tlix -= window;     // because we read in an additive sense.. 

	cout << "RIGHT NEIGHBOUR: " << endl;
	cout << "calling thingy with neighbour coords " << nlix << ", " << ny1 << "  this coords : " << tlix << ", " << y1 << "  size : " << areaW << ", " << areaH << endl;
	offsets off = findNeighbourOffset(rightNeighbour, neighborArea, nlix, ny1, thisArea, tlix, y1, areaW, areaH);
	cout << "\tadjustNeighbours off dx " << off.dx << "\toff dy " << off.dy << "\toff corr " << off.corr << endl;
	overlaps.push_back(new overlap_data(x, y, rightNeighbour->x_pos(), rightNeighbour->y_pos(), px, py, px + 1, py, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
	if(off.corr > minCorr){
	    rightNeighbour->adjustPosition(off.dx, off.dy, RIGHT, off.corr);
	    vector<overlap_data*> olaps = rightNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px+1, py);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}

// 	if(rightNeighbour->readToFloatPro(neighborArea, nlix, areaW, ny1, areaH, waveIndex)
// 	   &&
// 	   readToFloatPro(thisArea, tlix, areaW, y1, areaH, waveIndex))

// //	if(rightNeighbour->readToFloat(neighborArea, nlix, areaW, ny1, areaH, secNo, wavelength, 1.0)
// //	   &&
// //	   readToFloat(thisArea, tlix, areaW, y1, areaH, secNo, wavelength, 1.0))

// 	{
// 	    offsets off = iFunc.findOffsets(thisArea, neighborArea, areaW, areaH);
// 	    overlaps.push_back(new overlap_data(x, y, rightNeighbour->x_pos(), rightNeighbour->y_pos(), px, py, px + 1, py, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
// 	    if(off.corr > minCorr){
// 		rightNeighbour->adjustPosition(off.dx, off.dy, RIGHT, off.corr);
// 		vector<overlap_data*> olaps = rightNeighbour->adjustNeighbourPositions(secNo, wavelength, rolloff, instep, window, px+1, py);
// 		overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
// 	    }
// 	}else{
// 	    cerr << "FrameStack::adjustNeighbour unable to read into areas for checking right overlap" << endl;
// 	    delete neighborArea;
// 	    delete thisArea;
// 	}
    }    
    
    if(topNeighbour && !topNeighbour->adjustedNeighbours()){
//    if(topNeighbour && !(topNeighbour->isPositionAdjusted())){
	
	int gy = (top()  + topNeighbour->bottom())/2;
//	int gy = pixelY + pHeight - rolloff - instep - window;  

	int yb = gy - bottom();
	int nyb = gy - topNeighbour->bottom();
//	int ww = window * 2 + 1;
	int gx1 = pixelX + rolloff + margin;
	int gx2 = pixelX + pWidth - rolloff - margin;
	int x1 = gx1 - left();
	int nx1 = gx1 - topNeighbour->left();

	if( (window + yb) >= (pHeight - rolloff)){
	    cout << "adjustNeighbourPositions (topNeighbour) : window changing from " << window;
	    window = ((pHeight - rolloff) - yb);
	    cout << "\tto : " << window << endl;
	}
	if(window <= 0){
	    cerr << "adjustNeighbourPositions window is less than 0 : " << window << endl;
	    return(overlaps);
	}

	int areaW = gx2 - gx1;
	int areaH = window * 2 + 1;

	yb -= window;
	nyb -= window;

	float* neighborArea = new float[areaH * areaW];
	float* thisArea = new float[areaH * areaW];
	
	cout << "TOPNEIGHBOUR" << endl;
	offsets off = findNeighbourOffset(topNeighbour, neighborArea, nx1, nyb, thisArea, x1, yb, areaW, areaH);
	cout << "\tadjustNeighbours off dx " << off.dx << "\toff dy " << off.dy << "\toff corr " << off.corr << endl;
	overlaps.push_back(new overlap_data(x, y, topNeighbour->x_pos(), topNeighbour->y_pos(), px, py, px, py+1, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
	if(off.corr > minCorr){
	    topNeighbour->adjustPosition(off.dx, off.dy, TOP, off.corr);
	    vector<overlap_data*> olaps = topNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px, py+1);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}
	
// 	if(topNeighbour->readToFloatPro(neighborArea, nx1, areaW, nyb, areaH, waveIndex)
// 	   &&
// 	   readToFloatPro(thisArea, x1, areaW, yb, areaH, waveIndex))
// //	if(topNeighbour->readToFloat(neighborArea, nx1, areaW, nyb, areaH, secNo, wavelength, 1.0)
// //	   &&
// //	   readToFloat(thisArea, x1, areaW, yb, areaH, secNo, wavelength, 1.0))
// 	{
// 	    offsets off = iFunc.findOffsets(thisArea, neighborArea, areaW, areaH);
// 	    cout << "\tTOP neighbor iFunc offset : dx " << off.dx << "  dy : " << off.dy << "   correlation : " << off.corr << "  --> " << off.norm_corr << endl;
// 	    overlaps.push_back(new overlap_data(x, y, topNeighbour->x_pos(), topNeighbour->y_pos(), px, py, px, py+1, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
// 	    if(off.corr > minCorr){
// 		topNeighbour->adjustPosition(off.dx, off.dy, TOP, off.corr);
// 		vector<overlap_data*> olaps = topNeighbour->adjustNeighbourPositions(secNo, wavelength, rolloff, instep, window, px, py+1);
// 		overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
// 	    }
// 	}else{
// 	    delete neighborArea;
// 	    delete thisArea;
// 	}
	
    }
    if(leftNeighbour && !leftNeighbour->adjustedNeighbours()){
//    if(leftNeighbour && !(leftNeighbour->isPositionAdjusted())){
	
	int gx = (left() + leftNeighbour->right())/2;
	
//	int gx = left() + rolloff + instep - window;
	int x1 = gx - left();
	int nx1 = gx - leftNeighbour->left();
	int gy1 = pixelY + rolloff + margin;
	int gy2 = pixelY + pHeight - rolloff - margin;
	int y1 = rolloff + margin;
	int ny1 = gy1 - leftNeighbour->bottom();

	if( (x1 - window) <= rolloff){
	    cout << "adjustNeighbourPositions : window changing from " << window;
	    window = (x1 - rolloff);
	    cout << "\tto : " << window << endl;
	}
	if(window <= 0){
	    cerr << "adjustNeighbourPositions window is less than 0 : " << window << endl;
	    return(overlaps);
	}
	
	int areaW = window * 2 + 1;
	int areaH = gy2 - gy1;

	x1 -= window;
	nx1 -= window;

	float* neighborArea = new float[areaW * areaH];
	float* thisArea = new float[areaW * areaH];

	cout << "LEFTNEIGHBOUR" << endl;
	offsets off = findNeighbourOffset(leftNeighbour, neighborArea, nx1, ny1, thisArea, x1, y1, areaW, areaH);
	cout << "\tadjustNeighbours off dx " << off.dx << "\toff dy " << off.dy << "\toff corr " << off.corr << endl;
	overlaps.push_back(new overlap_data(x, y, leftNeighbour->x_pos(), leftNeighbour->y_pos(), px, py, px-1, py, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
	if(off.corr > minCorr){
	    leftNeighbour->adjustPosition(off.dx, off.dy, LEFT, off.corr);
	    vector<overlap_data*> olaps = leftNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px-1, py);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}

// 	if(leftNeighbour->readToFloatPro(neighborArea, nx1, areaW, ny1, areaH, waveIndex)
// 	   &&
// 	   readToFloatPro(thisArea, x1, areaW, y1, areaH, waveIndex))
// //	if(leftNeighbour->readToFloat(neighborArea, nx1, areaW, ny1, areaH, secNo, wavelength, 1.0)
// //	   &&
// //	   readToFloat(thisArea, x1, areaW, y1, areaH, secNo, wavelength, 1.0))
// 	{
// 	    offsets off = iFunc.findOffsets(thisArea, neighborArea, areaW, areaH);
// 	    overlaps.push_back(new overlap_data(x, y, leftNeighbour->x_pos(), leftNeighbour->y_pos(), px, py, px-1, py, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
// 	    if(off.corr > minCorr){
// 		leftNeighbour->adjustPosition(off.dx, off.dy, LEFT, off.corr);
// 		vector<overlap_data*> olaps = leftNeighbour->adjustNeighbourPositions(secNo, wavelength, rolloff, instep, window, px-1, py);
// 		overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
// 	    }
// 	}else{
// 	    delete neighborArea;
// 	    delete thisArea;
// 	}
    }
    
    if(bottomNeighbour && !bottomNeighbour->adjustedNeighbours()){
//    if(bottomNeighbour && !(bottomNeighbour->isPositionAdjusted())){
	
	int gy = (bottom() + bottomNeighbour->top())/2;

//	int gy = pixelY + rolloff + instep - window;

	int y1 = gy - bottom();
	int ny1 = gy - bottomNeighbour->bottom();

	int gx1 = pixelX + rolloff + margin;
	int gx2 = pixelX + pWidth - rolloff - margin;
	
	int x1 = rolloff + margin;
	int nx1 = gx1 - bottomNeighbour->left();

	if( (y1 - window) <= rolloff){
	    cout << "adjustNeighbourPositions : window changing from " << window;
	    window = (y1 - rolloff);
	    cout << "\tto : " << window << endl;
	}
	if(window <= 0){
	    cerr << "adjustNeighbourPositions window is less than 0 : " << window << endl;
	    return(overlaps);
	}

	
	int areaW = gx2 - gx1;
	int areaH = window * 2 + 1;

	y1 -= window;
	ny1 -= window;
	
	float* neighbourArea = new float[areaW * areaH];
	float* thisArea = new float[areaW * areaH];
	
	cout << "BOTTOMNEIGHBOUR" << endl;
	offsets off = findNeighbourOffset(bottomNeighbour, neighbourArea, nx1, ny1, thisArea, x1, y1, areaW, areaH);
	cout << "\tadjustNeighbours off dx " << off.dx << "\toff dy " << off.dy << "\toff corr " << off.corr << endl;
	overlaps.push_back(new overlap_data(x, y, bottomNeighbour->x_pos(), bottomNeighbour->y_pos(), px, py, px, py-1, areaW, areaH, off.dx, off.dy, thisArea, neighbourArea, off));
	if(off.corr > minCorr){
	    bottomNeighbour->adjustPosition(off.dx, off.dy, BOTTOM, off.corr);
	    vector<overlap_data*> olaps = bottomNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px, py-1);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}

// 	if(bottomNeighbour->readToFloatPro(neighbourArea, nx1, areaW, ny1, areaH, waveIndex)
// 	   &&
// 	   readToFloatPro(thisArea, x1, areaW, y1, areaH, waveIndex))
// //	if(bottomNeighbour->readToFloat(neighbourArea, nx1, areaW, ny1, areaH, secNo, wavelength, 1.0)
// //	   &&
// //	   readToFloat(thisArea, x1, areaW, y1, areaH, secNo, wavelength, 1.0))
// 	{
// 	    offsets off = iFunc.findOffsets(thisArea, neighbourArea, areaW, areaH);
// 	    overlaps.push_back(new overlap_data(x, y, bottomNeighbour->x_pos(), bottomNeighbour->y_pos(), px, py, px, py-1, areaW, areaH, off.dx, off.dy, thisArea, neighbourArea, off));
// 	    if(off.corr > minCorr){
// 		bottomNeighbour->adjustPosition(off.dx, off.dy, BOTTOM, off.corr);
// 		vector<overlap_data*> olaps = bottomNeighbour->adjustNeighbourPositions(secNo, wavelength, rolloff, instep, window, px, py-1);
// 		overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
// 	    }
// 	}else{
// 	    delete neighbourArea;
// 	    delete thisArea;
//	}
    }
    
    return(overlaps);
}

offsets FrameStack::findNeighbourOffset(FrameStack* neighbour, float* neighbourArea, int nx1, int ny1, float* thisArea, int x1, int y1, int& areaW, int& areaH){
    ImageFunc iFunc;
    offsets off;  // in case nothing works..

    // first of all check that the areas make sense...
    int minY = y1 < ny1 ? y1 : ny1;
    int minX = x1 < nx1 ? x1 : nx1;

    if(minY < 0){
	ny1 -= minY;
	y1 -= minY;
	areaH += minY;
    }
    if(minX < 0){
	nx1 -= minX;
	x1 -= minX;
	areaW += minX;
    }

    // since we need to make sure that 
    // neighbourArea and thisArea are set to the appropriate thingy we need to remember 
    // this is necessary for the overlaps thingy.. to do things appropriately..
    float* n_tempArea = new float[areaW * areaH];
    float* tempArea = new float[areaW * areaH];
    int maxIndex = -1;   // if it never gets set then we should rescue it..  
//    cout << "findNeighbourOffset " << endl;
    for(int i=1; i < wave_no; ++i){
	if(neighbour->readToFloatPro(n_tempArea, nx1, areaW, ny1, areaH, i)
	   && readToFloatPro(tempArea, x1, areaW, y1, areaH, i))
	{
	    offsets new_off = iFunc.findOffsets(tempArea, n_tempArea, areaW, areaH);
	    cout << i << "\t new_off.corr " << new_off.corr << "\tnorm_corr : " << new_off.norm_corr << "\tnew_off.dx " << new_off.dx << "\tnew_off.dy " << new_off.dy << endl; 
	    if(new_off.corr > off.corr){
		off = new_off;
		maxIndex = i;
		memcpy(thisArea, tempArea, sizeof(float) * areaW * areaH);
		memcpy(neighbourArea, n_tempArea, sizeof(float) * areaW * areaH);
	    }
	}
    }
    if(maxIndex == -1){                                           // though this shouldn't really ever happen.. since the off.corr defaults to -2
	memcpy(thisArea, tempArea, sizeof(float) * areaW * areaH);
	memcpy(neighbourArea, n_tempArea, sizeof(float) * areaW * areaH);
    }
    delete n_tempArea;
    delete tempArea;
//    cout << "\tfinally off.corr " << off.corr << "\toff.dx " << off.dx << "\toff.dy " << endl;
    return(off);
}

bool FrameStack::readToRGB(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int dest_pwidth, unsigned int dest_pheight, unsigned int slice_no, 
			   float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, bool bg_sub, raw_data* raw)
{
    if(!finalised){
	cerr << "FrameStack::readToRGB Trying to read into buffer from unfinalised framestack. Will finalise, but this suggests error in code" << endl;
	finalise(maxLevel);
    }
    if(slice_no >= sections.size()){
	cerr << "FrameStack::readToRGB slice_no larger then sections size. slice_no : " << slice_no << " sections size : " << sections.size() << endl;
	return(false);
    }
    // in fact since we are only going to assign from the non-overlapping region, well 1/2 the overlapping region..
//    cout << "\tFrameStack::readToRGB overlaps : left " << leftOverlap << "   right : " << rightOverlap << "  bottom " << bottomOverlap << "  top " << topOverlap << endl;
    float lx1 = x + leftOverlap/2.0;
    float lx2 = x + width - rightOverlap/2.0;
    float ly1 = y + bottomOverlap/2.0;
    float ly2 = y + height - topOverlap/2.0;

    // at this point we need to determine which pixels we want to add to the thingy..
    if(!( lx2 > xpos && lx1  < (xpos + dest_width) ) ){
	return(false);
    }
    if(!( ly2 > ypos && ly1 < (ypos + dest_height) ) ){
	return(false);
    }
    
    //cout << "\tFrameStack::readToRGB  data requested from " << xpos << " --> " << xpos + dest_width << " local coverage " << lx1 << " --> " << lx2 << "  pixelX " << pixelX << " pwidth : " << pWidth << endl;
       //cout << "\tFrameStack::readToRGB  data requested from " << ypos << " --> " << ypos + dest_height << " local coverage " << ly1 << " --> " << ly2 << "  pixelY " << pixelY << " pheight : " << pHeight << endl;
    //cout << "\tFrame boundaries set at  horizonta : " << leftBorder << " --> " << rightBorder << " (" << leftBorder - pixelX << " -> " << rightBorder - pixelX << ")  "
//	 << "vertical : " << bottomBorder << " --> " << topBorder << "  (" << bottomBorder - pixelY << " -> " << topBorder - pixelY << ")" << endl;

    // so now we have an overlap.. we have to work out the local coordinates of that overlap..
    float x1, x2, y1, y2;
    x1 = xpos < lx1 ? lx1 : xpos;
    x2 = (xpos + dest_width) > lx2 ? lx2 : (xpos + dest_width);
    y1 = ypos < ly1 ? ly1 : ypos;
    y2 = (ypos + dest_height) > ly2 ? ly2 : (ypos + dest_height);

    /// now all we need to do is to translate this into pixel positions, both in the rgb buffer (dest) and in the local buffer.
    /// then just call the underlying functions and we should be fine..
    float dx = float(pWidth)/width;
    float dy = float(pHeight)/height;
    unsigned int ix, iw, iy, ih, dix, diy;     // i for int, but d for destination
    ix = (unsigned int)((x1 - x) * dx);
    iw = (unsigned int)((x2 - x1) * dx);
    iy = (unsigned int)((y1 - y) * dy);
    ih = (unsigned int)((y2 - y1) * dy);

    dix = (unsigned int)((x1 - xpos) * dx);
    diy = (unsigned int)((y1 - ypos) * dy);   // d stands for destiation.. 

    

    // by definition, the scale of the the two have to be the same, -anything else would be too difficult to handle
    
    // make sure that we are not going out of bounds..
    if(dix + iw > dest_pwidth){
	cerr << "FrameStack::readToRGB dix + iw > dest_pwidth adjusting by subtracting " << (dest_pwidth - (dix + iw)) << "  from  " << iw << endl;
	iw = dest_pwidth - dix;   // which should make things ok ..
    }
    if(diy + ih > dest_pheight){
	cerr << "FrameStack::readToRGB diy + h > dest_pheight adjusting by subtracting " << (dest_pheight - (diy + ih)) << "  from " << ih << endl;
	ih = dest_pheight - diy;
    }
    
    // and then we should be able to call the relevant function in the appropriate fileset..
    //   cout << "\n call Function with " << ix << ", " << iy << ", " << iw << ", " << ih << ", " << dix << ", " << diy << ", " << dest_pwidth << endl;
    return(sections[slice_no]->readToRGB(dest, ix, iy, iw, ih, dix, diy, dest_pwidth, maxLevel, bias, scale, colors, raw));
    
}

bool FrameStack::readToRGB(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height, unsigned int slice_no, 
			   float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, bool bg_sub, raw_data* raw){
//     if(pixelX == -1 && pixelY == -1){
// 	cerr << "FrameStack::readToRGB (int version) pixel positions undefined " << endl;
// 	return(false);
//     }
    if(slice_no >= sections.size()){
	cerr << "FrameStack::readToRGB (int version) slice no is too large : " << slice_no << endl;
	return(false);
    }
    int dest_x, source_x, dest_y, source_y;
    unsigned int subWidth, subHeight;
    // check if it overlaps with the border positions..
        if(globalToLocal(xpos, ypos, dest_width, dest_height, dest_x, source_x, dest_y, source_y, subWidth, subHeight)){
//     if(
// 	(xpos < rightBorder && xpos + int(dest_width) > leftBorder)  // overlap in horizontal direction 
// 	&&
// 	(ypos < topBorder && ypos + int(dest_height) > bottomBorder)  // overlap in vertical plane ?
// 	){
	
// 	// then first work out which pixels to take from here, and which pixels to put them to.. 
// 	int dest_x = leftBorder >= xpos ? leftBorder - xpos : 0;
// 	int source_x = leftBorder >= xpos ? leftBorder - pixelX : xpos - pixelX;
// 	int dest_y = bottomBorder >= ypos ? bottomBorder - ypos : 0;
// 	int source_y = bottomBorder >= ypos ? bottomBorder - pixelY : ypos - pixelY;


// 	if(source_x < 0){
// 	    dest_x -= source_x;
// 	    source_x = 0;
// 	}
// 	if(source_y < 0){
// 	    dest_y -= source_y;
// 	    source_y = 0;
// 	}

// 	// and then work out the width and height that we'll be taking out of this.. 
// 	unsigned int subWidth = rightBorder <= xpos + int(dest_width) ? (rightBorder - pixelX) - source_x : (xpos + dest_width) - leftBorder;
// 	unsigned int subHeight = topBorder <= ypos + int(dest_width) ? (topBorder - pixelY) - source_y : (ypos + dest_height) - bottomBorder;


	// because borders are inclusive..
	//subWidth++;
	//subHeight++;

	// and print out these numbers to see if they make much sense.. 
// 	cout << "FrameStack::readToRGB (int version) \n"
// 	     << "\trequest : " << xpos << ", " << ypos << " --> " << xpos + dest_width << ", " << ypos + dest_height 
// 	     << "  border positions " << leftBorder << ", " << bottomBorder << " --> " << rightBorder << ", " << topBorder << endl
// 	     << "\tinternal positions  dest : " << dest_x << ", " << dest_y << "     source : " << source_x << ", " << source_y << endl
// 	     << "\t\tpixel position: " << pixelX << "," << pixelY <<  "    dims : " << subWidth << ", " << subHeight << endl;
	
// 	cout << "\n call Function with " << source_x << ", " << source_y << ", " << subWidth  << ", " << subHeight << ", " << dest_x << ", " << dest_y << ", " << dest_width << endl;
//	cout << "Recorded Z position : " << sections[slice_no]->z_pos() << endl;
	return(sections[slice_no]->readToRGB(dest, source_x, source_y, subWidth, subHeight, dest_x, dest_y, dest_width, maxLevel, bias, scale, colors, bg_sub, raw));
    }
    return(false);
}


bool FrameStack::mip_projection(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int dest_pwidth, unsigned int dest_pheight, 
				float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw)
{

    // for each colour make a float array containing the maximum intensity at that x-y position
    // then add to dest in the approrpriate manner (check out frame.cpp to see how to do this).

    // Frist work out where to get the numbers from .. 

    if(!finalised){
	cerr << "FrameStack::readToRGB Trying to read into buffer from unfinalised framestack. Will finalise, but this suggests error in code" << endl;
	finalise(maxLevel);
    }

    float lx1 = x + leftOverlap/2.0;
    float lx2 = x + width - rightOverlap/2.0;
    float ly1 = y + bottomOverlap/2.0;
    float ly2 = y + height - topOverlap/2.0;
    
    // at this point we need to determine which pixels we want to add to the thingy..
    if(!( lx2 > xpos && lx1  < (xpos + dest_width) ) ){
	return(false);
    }
    if(!( ly2 > ypos && ly1 < (ypos + dest_height) ) ){
	return(false);
    }

        // so now we have an overlap.. we have to work out the local coordinates of that overlap..
    float x1, x2, y1, y2;
    x1 = xpos < lx1 ? lx1 : xpos;
    x2 = (xpos + dest_width) > lx2 ? lx2 : (xpos + dest_width);
    y1 = ypos < ly1 ? ly1 : ypos;
    y2 = (ypos + dest_height) > ly2 ? ly2 : (ypos + dest_height);

    /// now all we need to do is to translate this into pixel positions, both in the rgb buffer (dest) and in the local buffer.
    /// then just call the underlying functions and we should be fine..
    float dx = float(pWidth)/width;
    float dy = float(pHeight)/height;
    unsigned int ix, iw, iy, ih, dix, diy;     // i for int, but d for destination
    ix = (unsigned int)((x1 - x) * dx);
    iw = (unsigned int)((x2 - x1) * dx);
    iy = (unsigned int)((y1 - y) * dy);
    ih = (unsigned int)((y2 - y1) * dy);

    dix = (unsigned int)((x1 - xpos) * dx);
    diy = (unsigned int)((y1 - ypos) * dy);   // d stands for destiation.. 

    // by definition, the scale of the the two have to be the same, -anything else would be too difficult to handle
    
    // make sure that we are not going out of bounds..
    if(dix + iw > dest_pwidth){
	cerr << "FrameStack::readToRGB dix + iw > dest_pwidth adjusting by subtracting " << (dest_pwidth - (dix + iw)) << "  from  " << iw << endl;
	iw = dest_pwidth - dix;   // which should make things ok ..
    }
    if(diy + ih > dest_pheight){
	cerr << "FrameStack::readToRGB diy + h > dest_pheight adjusting by subtracting " << (dest_pheight - (diy + ih)) << "  from " << ih << endl;
	ih = dest_pheight - diy;
    }
    
    // and then make the two buffers that we need..
    float* buffer = new float[iw * ih];
    float* mip_buffer = new float[iw * ih];

    cout << "made buffers with size " << iw << " * " << ih << " = " << iw * ih << endl;
   
    bool ok = true;
    // and then go through the thingys and set stuff up..
    for(uint i=0; i < fwaves.size(); i++){
//	cout << "FrameStack::mip_projection wavelength : " << i << " : " << fwaves[i] << endl;
	memset(buffer, 0, sizeof(float) * iw * ih);
	memset(mip_buffer, 0, sizeof(float) * iw * ih);
	for(uint j=0; j < sections.size(); j++){
//	    cout << "\tsection : " << j << endl;
	  ///////// CHANGE HERE TO REMOVE BACKGROUND CORRECTION, OR TO MAKE OPTIONAL.. FOR PROJECTION 
	  if(!sections[j]->readToFloat(buffer, ix, iy, iw, ih, 0, 0, iw, maxLevel, i)){
//	    if(!sections[j]->readToFloat(buffer, 0, 0, iw, ih, 0, 0, iw, maxLevel, fwaves[i])){
		ok = false;
		cerr << "FrameStack::mip_projection unable to read from section : " << j << "  wavelength : " << fwaves[i] << " (" << i << ")" << endl;
	    }else{
		for(uint k=0; k < iw * ih; k++){
		    mip_buffer[k] = mip_buffer[k] < buffer[k] ? buffer[k] : mip_buffer[k];
		}
	    }
	}
	/// and at this point we have to map the positions in the mip_buffer to the destination buffer.. this is a bit of a pain to say the least..
	/// and we have to convert all the values like we normally do..
//	cout << "Made the mip_buffer  now let's map to original " << endl;
	float* source;
	float* dst;
	float* rdst = 0;  // this is the raw destination.. 
	for(unsigned int yp = 0; yp < ih; yp++){
	    source = mip_buffer + yp * iw;
	    dst = dest + (diy * dest_pwidth + dix + yp * dest_pwidth) * 3;
	    if(raw){
		rdst = raw->values[i] + (diy * dest_pwidth + dix + yp * dest_pwidth);
	    }
	    for(unsigned int xp = 0; xp < iw; xp++){
		float v = bias[i] + scale[i] * (*source);
		if(v > 0){
		    dst[0] += v * colors[i].r;
		    dst[1] += v * colors[i].g;
		    dst[2] += v * colors[i].b;
		}
		if(raw){
		    *rdst = (*source);
		}
		++rdst;
		++source;
		dst += 3;
	    }
	}
    }
    cout << "about to delete buffer " << endl;
    delete buffer;
    delete mip_buffer;
    return(ok);
}

bool FrameStack::mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
				float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, raw_data* raw){
    if(!projection){
	cout << "FrameStack::mip_projection projection is 0 so making new projection " << endl;
	finalise(maxLevel);
    }

     cout << "FrameStack::mip_projection : request :" << xpos << ", " << ypos << "  width, height : " << dest_width << ", " << dest_height
 	 << " source : " << pixelX << ", " << pixelY << "  leftBorder : " << leftBorder << "  rightBorder : " << rightBorder << "  topBorder "
 	 << topBorder << "  bottomBorder " << bottomBorder << endl;

    int dest_x, source_x, dest_y, source_y;
    unsigned int subWidth, subHeight;

    if(globalToLocal(xpos, ypos, dest_width, dest_height, dest_x, source_x, dest_y, source_y, subWidth, subHeight)){
//     if(
// 	(xpos < rightBorder && xpos + int(dest_width) > leftBorder)  // overlap in horizontal direction 
// 	&&
// 	(ypos < topBorder && ypos + int(dest_height) > bottomBorder)  // overlap in vertical plane ?
// 	){
// 	// then first work out which pixels to take from here, and which pixels to put them to.. 
// 	int dest_x = leftBorder >= xpos ? leftBorder - xpos : 0;
// 	int source_x = leftBorder >= xpos ? leftBorder - pixelX : xpos - pixelX;
// 	int dest_y = bottomBorder >= ypos ? bottomBorder - ypos : 0;
// 	int source_y = bottomBorder >= ypos ? bottomBorder - pixelY : ypos - pixelY;
// 	if(source_x < 0){
// 	    dest_x -= source_x;
// 	    source_x = 0;
// 	}
// 	if(source_y < 0){
// 	    dest_y -= source_y;
// 	    source_y = 0;
// 	}
// 	// and then work out the width and height that we'll be taking out of this.. 
// 	unsigned int subWidth = rightBorder <= xpos + int(dest_width) ? (rightBorder - pixelX) - source_x : (xpos + dest_width) - leftBorder;
// 	unsigned int subHeight = topBorder <= ypos + int(dest_width) ? (topBorder - pixelY) - source_y : (ypos + dest_height) - bottomBorder;
	// and then just map from our projections ..
	float v;
	cout << "\toverlap found , dest_pos : " << dest_x << ", " << dest_y << "  source : " << source_x << ", " << source_y << "  dimensions : " << subWidth << ", " << subHeight << endl;
	for(uint wi=0; wi < fwaves.size(); ++wi){
	    for(uint hp=0; hp < subHeight; ++hp){
		float* dst = dest + ((dest_y + hp) * dest_width + dest_x) * 3;   // dest is an rgb triplet..
		float* src = projection[wi] + (source_y + hp) * pWidth + source_x;
		float* raw_dest = 0;
		if(raw){
		    raw_dest = raw->values[wi] + (dest_y + hp) * dest_width + dest_x;
		}
		for(uint wp=0; wp < subWidth; ++wp){
		    // and do stuff..
		    if(raw){
			(*raw_dest) = (*src);
			++raw_dest;
		    }
		    v = bias[wi] + (*src) * scale[wi];
		    if(v > 0){
			dst[0] += v * colors[wi].r;
			dst[1] += v * colors[wi].g;
			dst[2] += v * colors[wi].b;
		    }
		    ++src;
		    dst += 3;
		}
	    }
	}
	return(true);
    }
    return(false);
}

bool FrameStack::readToFloatPro(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, 
				unsigned int iheight, unsigned int wave)
{
    cout << "readToFloatPro " << xb << ", " << yb << " :  " << iwidth << ", " << iheight << endl;
    if(!projection){
	finalise(maxIntensity);
    }

    if(xb + iwidth >= pWidth || yb + iheight >= pHeight){
	cerr << "FrameStack::ReadToFloatPro coordinates are larger than projection (remember to use local coordinates not global) "
	     << xb << ", " << yb << " : " << iwidth << ", " << iheight << endl;
	return(false);
    }

    if(wave >= wave_no){
	cerr << "FrameStack::ReadToFloatPro wave is larger than wave_no " << wave << " >= " << wave_no << endl;
	return(false);
    }

    for(uint yp = 0; yp < iheight; ++yp){
	float* src = projection[wave] + (yb + yp) * pWidth + xb;
	float* dst = dest + yp * iwidth;
	memcpy((void*)dst, (const void*)src, sizeof(float) * iwidth);
    }
    return(true);

}
    
bool FrameStack::readToFloatProGlobal(float* dest, int xb, int iwidth, int yb, int iheight, unsigned int wave){
  cout << "FrameStack::readToFloatProGlobal " << endl;
    int dest_x, source_x, dest_y, source_y;
    unsigned int subWidth, subHeight;
    if(wave >= fwaves.size()){
	cerr << "FrameStack::readToFloatGlobal wave index specified is too large.. " << wave << endl;
	return(false);
    }
    if(!globalToLocal(xb, yb, iwidth, iheight, dest_x, source_x, dest_y, source_y, subWidth, subHeight)){
	return(false);
    }
    float* dst = dest;
    bool ok = true;
//    cout << "FrameStack::readToFloatProGlobal requested : " << xb << " +-> " << iwidth << " , " << yb << " +-> " << iheight << "  giving " << source_x << " +-> " << subWidth << " , "
//	 << source_y << " +-> " << subHeight << endl;
    for(uint yp = 0; yp < subHeight; ++yp){
	float* src = projection[wave] + (source_y + yp) * pWidth + source_x;
	float* dst = dest + (dest_y + yp) * iwidth + dest_x;
	memcpy((void*)dst, (const void*)src, sizeof(float) * subWidth);
    }
    return(true);
}

float* FrameStack::make_mip_projection(unsigned int wi, float maxLevel, vector<float>& contrast){
    unsigned int margin = 32;  // but we should be supplying this from somewhere else ? -ideally we should make it a property of the frameStack .. 
                               // actually this is the rolloff and I need to fix this somewhere.. 
    float* proj = new float[pWidth * pHeight];
    memset((void*)proj, 0, sizeof(float) * pWidth * pHeight);
    float* buf = new float[pWidth * pHeight];
    contrast.resize(sections.size());
    cout << "FrameStack::make_mip_projection" << endl;
    for(uint i=0; i < sections.size(); ++i){
      if(sections[i]->readToFloat(buf, 0, 0, pWidth, pHeight, 0, 0, pWidth, maxLevel, wi)){
	    // check if we need to update the buffers..
	    for(uint j=0; j < pWidth * pHeight; ++j){
		proj[j] = buf[j] > proj[j] ? buf[j] : proj[j];
	    }
	    // and then do the contrasts..
	    contrast[i] = determineContrast(buf, pWidth, pHeight, margin, margin, pWidth - 2 * margin, pHeight - 2 * margin);
	}
    }
    //    for(uint j=0; j < pWidth; ++j){
    //  cout << j << " : " << proj[ 512 * pWidth + j] << endl;
    //}
    return(proj);
}

bool FrameStack::readToFloat(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, unsigned int iheight, unsigned int secNo, unsigned int waveIndex, float maxLevel){
  cout << "FrameStackk::readToFloat" << endl;
    if(secNo >= sections.size()){
	cerr << "FrameStack::readToFloat secNo (" << secNo << ") is larger than sections size : " << sections.size() << endl;
	return(false);
    }
    return(sections[secNo]->readToFloat(dest, xb, yb, iwidth, iheight, 0, 0, iwidth, maxLevel, waveIndex));
}

bool FrameStack::readToFloat(float* dest, int xb, int iwidth, int yb, 
		 int iheight, int zb, int idepth,  unsigned int waveIndex, float maxLevel){   // simply read the appropriate pixels into a volume.. 
  cout << "FrameStack::readToFloat number 2" << endl;
    // check that zb and idepth are ok.. zb must be > 0 and < sections.size..
    if(zb < 0 || zb >= int(sections.size()) || idepth <= 0 || zb + idepth > int(sections.size())){
	cerr << "FrameStack unsuitable z-sections requested : " << zb << "  -->  " << zb + idepth << endl;
	return(false);
    }
    
    int dest_x, source_x, dest_y, source_y;      // though these are supposed to be unsigned.. (I really should change everything everywhere to signed)
    unsigned int subWidth, subHeight;   // the local coordinates of an overlap for dest and source respectively..
    if(!globalToLocal(xb, yb, iwidth, iheight, dest_x, source_x, dest_y, source_y, subWidth, subHeight)){
//	cout << "*  " << topBorder;
	return(false);   // this just means that there is no overlap and there is nothing more for us to do here..
    }
    
    float* dst = dest;
    // then just go through the z sections and call readToFloat on each one..
    bool ok = true;
    for(int zp=zb; zp < zb + idepth; ++zp){
      if(!sections[zp]->readToFloat(dst, source_x, source_y, subWidth, subHeight, dest_x, dest_y, iwidth, maxIntensity, waveIndex)){
	    ok = false;
//	    cout << "-//\\-" << endl;
	}
	dst += iwidth * iheight;
    }
    return(ok);    
}

void FrameStack::normalise_y(float* values, unsigned int w, unsigned int h){
    // assume values contains the approriate things..
    for(uint xp=0; xp < w; xp++){
//	cout << "xp " << xp << endl;
	float Sq = 0;  // squared sum
	float S = 0;   // sum
	for(uint yp=0; yp < h; yp++){
	    uint p = yp * w + xp;
	    Sq += (values[p] * values[p]);
	    S += values[p];
	}
	float n = (float)h;
	float std =  sqrt( (Sq - S*S/n)/(n - 1.0) );
	float mean = S / n;
//	cout << "std " << std << "  mean " << mean << " Sq " << Sq << " S " << S << "  and S*S " << S*S << "  and S^2 - Sq  " << Sq - S*S << endl;
	// and then go through an normalise the values..
	for(uint yp = 0; yp < h; yp++){
	    uint p = yp * w + xp;
//	    cout << "\t yp : " << yp << "\t p : "<< p << endl;
	    values[p] = (values[p] - mean) / std;
	}
    }
    cout << "end of normalise_y" << endl;
}

void FrameStack::normalise_x(float* values, unsigned int w, unsigned int h){
    // assume values contains the approriate things..
    for(uint yp=0; yp < h; yp++){
	double Sq = 0;  // squared sum
	double S = 0;   // sum
	for(uint xp=0; xp < w; xp++){
	    uint p = yp * w + xp;
	    Sq += (values[p] * values[p]);
	    S += values[p];
	}
	double n = (float)w;
	double std =  sqrt( (Sq - S*S/n)/(n - 1.0) );
	double mean = S / n;
//	mean = mean * 2;
//	cout << "normalise_x n : " << n << "  std : " << std << "  mean : " << mean << "  S : " << S << "  Sq : " << Sq << endl;
	double chkSum = 0;
	// and then go through an normalise the values..
	for(uint xp = 0; xp < w; xp++){
	    uint p = yp * w + xp;
	    values[p] = (values[p] - mean) / std;
	    chkSum += values[p];
//	    cout << "  " << values[p];
	}
//	cout << endl << "And the chksum is : " << chkSum << endl;
    }
}

float FrameStack::correlate(float* a, float* b, unsigned int l){
    normalise_x(a, l, 1);
    normalise_x(b, l, 1);
    float score = 0;
    for(uint i=0; i < l; i++){
	score += a[i] * b[i];
    }
    score = score / float(l);
    return(score);
}

void FrameStack::adjustPosition(FrameStack* neibor, int dx, int dy, float corr){
    // if I have been adjusted then this takes precedence, but 
    // probably we should change this function.. 
    

}

void FrameStack::adjustPosition(int dx, int dy, POSITION n, float setAdjustmentFlag){
//    cout << "FrameStack::adjustPosition pixelX : " << pixelX << "  --> ";

    // we only do this if setAdjustmentFlag is larger than my own positionAdjusted..
    if(setAdjustmentFlag < positionAdjusted){
	return;
    }

    pixelX += dx;
//    cout << pixelX << "   pixelY : " << pixelY << "  --> ";
    pixelY += dy;
//    cout << pixelY << endl;
    
    if(frameInformation){
	frameInformation->xp = pixelX;
	frameInformation->yp = pixelY;
//	cout << "\t\t\tSETTING up the pixel Information" << endl;
    }

    // we might have to change the boundary positions, as these will change for the 
    // things on the edges.. 
    //
    // leftBoundary cannot be less than pixelX and so on..
    if(leftBorder < pixelX){
	leftBorder = pixelX;
    }
    if(rightBorder > pixelX + int(pWidth)){
	rightBorder = pixelX + pWidth;
    }
    if(bottomBorder < pixelY){
	bottomBorder = pixelY;
    }
    if(topBorder > pixelY + int(pHeight)){
	topBorder = pixelY + pHeight;
    }

//    cout << "\tleftBorder : " << leftBorder << "\trightBorder : " << rightBorder << "\tbottomBorder : " << bottomBorder << "\ttopBorder : " << topBorder << endl;

    // so this is real messy, but .. let's see how it works.. 

    // we only spread the adjustment in rows as there may be too many differences between rows depending on the order 

    switch(n){
	case RIGHT :
	    if(rightNeighbour && !(rightNeighbour->isPositionAdjusted())){
		rightNeighbour->adjustPosition(dx, dy, RIGHT, 0.0);
	    }
	    break;
	case LEFT :
	    if(leftNeighbour && !(leftNeighbour->isPositionAdjusted())){
		leftNeighbour->adjustPosition(dx, dy, LEFT, 0.0);
	    }
	    break;
	case TOP :
	    if(topNeighbour && !(topNeighbour->isPositionAdjusted())){
		//topNeighbour->adjustPosition(dx, dy, TOP, 0.0);
	    }
	    break;
	case BOTTOM :
	    if(bottomNeighbour && !(bottomNeighbour->isPositionAdjusted())){
		//bottomNeighbour->adjustPosition(dx, dy, BOTTOM, 0.0);
	    }
	    break;
	default :
	    cerr << "FrameStack::adjustPosition Unknown position " << endl;
    }
    
    if(setAdjustmentFlag){
	positionAdjusted = setAdjustmentFlag;
    }

//     if(topNeighbour){
// 	topNeighbour->adjustPosition(dx, dy);
//     }
//     if(rightNeighbour){
// 	rightNeighbour->adjustPosition(dx, dy);
//     }

//     x += dx;
//     y += dy;
//     cout << y << endl;
}


void FrameStack::determineFocalPlanes(unsigned int rolloff){
    
    // work out the coordinates of the thingy that I'm going to be reading from..
    unsigned int iwidth = pWidth - (2 * rolloff);
    unsigned int iheight = pHeight - (2 * rolloff);

    if(iwidth < 10 || iheight < 10){
	cerr << "FrameStack::determineFocalPlanes iwdith or iheight is bad : " << iwidth << ", " << iheight << endl;
	return;
    }
    
    //   int* maxPlanes = new int[iwidth * iheight];

    float* buffer = new float[iwidth * iheight * sections.size()];
//    float* projection = new float[iwidth * iheight];
    

    contrasts.resize(fwaves.size());
    for(uint i=0; i < contrasts.size(); i++){
	contrasts[i].resize(sections.size());
	for(uint j=0; j < contrasts[i].size(); j++){
	    contrasts[i][j] = 0;
	}
    }

    float* dest = buffer;
    for(uint i=0; i < fwaves.size(); i++){
//	cout << "determining contrasts for wavelength : " << fwaves[i] << endl;
	dest = buffer;
	for(uint j=0; j < sections.size(); j++){
	    readToFloat(dest, rolloff, iwidth, rolloff, iheight, j, fwaves[i], 2048.0);
	    dest += (iwidth * iheight);

	}
	dest = buffer;
	for(uint j=0; j < sections.size(); ++j){
	    contrasts[i][j] = 0;
	    for(uint yp=0; yp < iheight; ++yp){
		for(uint xp=0; xp < iwidth - 1; ++xp){
		    unsigned int p = yp * iwidth + xp;
		    contrasts[i][j] += ((dest[p] - dest[p+1]) * (dest[p] - dest[p+1]));
		}
	    }
	    dest += (iwidth * iheight);
	}

	float maxContrast = -1;
	unsigned int maxSection = 0;
	for(uint j=0; j < contrasts[i].size(); ++j){
	    if(maxContrast < contrasts[i][j]){
		maxContrast = contrasts[i][j];
		maxSection = j;
	    }
	}
	cout << "\t" << fwaves[i] << " : " << maxContrast << "\t" << maxSection << endl;
    }
    delete buffer;
    
}

float FrameStack::determineContrast(float* values, unsigned int w, unsigned int h, unsigned int xp, unsigned int yp, unsigned int sw, unsigned int sh){
    float contrast = 0;
    float* src = values;
    if(xp + sw > w || yp + sh > h){
	cerr << "FrameStack::determineContrast sub width and height seem a tad off : " << xp << ", " << yp << "  --> " << sw << ", " << sh << endl;
	return(contrast);
    }
    for(uint ypp = yp; ypp < yp + sh; ++ypp){
	src = values + ypp * w;
	for(uint xpp = 1 + xp; xpp < xp + sw; ++xpp){
	    contrast += ((src[xpp] - src[xpp-1]) * (src[xpp] - src[xpp-1]));
	}
    }
    return(contrast);
}

float FrameStack::meanValue(float* v, unsigned int l){
    float sum = 0;
    for(uint i=0; i < l; i++){
	sum += v[i];
    }
    return(sum / float(l));
}

bool FrameStack::globalToLocal(int xpos, int ypos, int dest_width, int dest_height, int& dest_x, int& source_x, int& dest_y, int& source_y, unsigned int& subWidth, unsigned int& subHeight){
    if(
	(xpos < rightBorder && xpos + int(dest_width) > leftBorder)  // overlap in horizontal direction 
	&&
	(ypos < topBorder && ypos + int(dest_height) > bottomBorder)  // overlap in vertical plane ?
	){
	
	// then first work out which pixels to take from here, and which pixels to put them to.. 
	dest_x = leftBorder >= xpos ? leftBorder - xpos : 0;
	source_x = leftBorder >= xpos ? leftBorder - pixelX : xpos - pixelX;
	dest_y = bottomBorder >= ypos ? bottomBorder - ypos : 0;
	source_y = bottomBorder >= ypos ? bottomBorder - pixelY : ypos - pixelY;
	
	
	if(source_x < 0){
	    dest_x -= source_x;
	    source_x = 0;
	}
	if(source_y < 0){
	    dest_y -= source_y;
	    source_y = 0;
	}
	
//	cout << "globalToLocal :: \n" << xpos << ", " << ypos << ", " << dest_width << ", " << dest_height << "\n-->  "
//	     << "  " << dest_x <<  ", " << source_x << ", " << dest_y << ", " << source_y << ", " << endl;
	
	// and then work out the width and height that we'll be taking out of this.. 
	subWidth = rightBorder <= xpos + int(dest_width) ? (rightBorder - pixelX) - source_x : (dest_width - dest_x);
	subHeight = topBorder <= ypos + int(dest_height) ? (topBorder - pixelY) - source_y : (dest_height - dest_y);

	// I can't think of a better way of putting this.. but there should be.
	// We also have to make sure that we don't overstep the size of the destination buffer.. 

//	subWidth = rightBorder <= xpos + int(dest_width) ? (rightBorder - pixelX) - source_x : (xpos + dest_width) - leftBorder;
//	subHeight = topBorder <= ypos + int(dest_height) ? (topBorder - pixelY) - source_y : (ypos + dest_height) - bottomBorder;
//	cout << "  and subHeight : " << subHeight << "  subWidth   : " << subWidth << endl;
	return(true);
    }
    return(false);
}
