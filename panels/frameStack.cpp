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
#include "frameSet.h"
#include "frame.h"
#include "../image/imageFunc.h"
#include "idMap.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdio.h>

using namespace std;

// somehow we need rolloff and margin, though to be honest I've quite forgotten what each one does
// 
//int rolloff = 36; 
// it seems that margin isn't actually necessary at the moment. It can well be 0.
// rolloff is used instead to determine positions using the contrib map.

FrameStack::FrameStack(int* waveLengths, int waveNo, ifstream* inStream, float maxLevel, int xy_margin){
    maxIntensity = maxLevel;
    wave_lengths = waveLengths;
    wave_no = waveNo;
    fwaves.resize(wave_no);
    channelOffsets.resize(wave_no);
    focalPlanes.resize(wave_no);
    frameInformation = 0;
    rolloff = xy_margin;
    margin = 0;      // keep as experimental. Make sure no bad consequences.
    //    margin = xy_margin > 0 ? xy_margin : 0;
    for(int i=0; i < wave_no; i++){
	fwaves[i] = float(wave_lengths[i]);
	focalPlanes[i] = 0;
    }
    sort(fwaves.begin(), fwaves.end());
    
    
    in = inStream;

    bleach_count = 0;
    leftNeighbour = topNeighbour = rightNeighbour = bottomNeighbour = 0;
    leftOverlap = topOverlap = rightOverlap = bottomOverlap = 0;
    real_x = real_y = 0;
    width = height = 0;
    z_begin = z_end =  0;
    pWidth = pHeight = 0;
    pixelX = pixelY = -1;    // which should be an undefined position.. 
    finalised = false;
    positionAdjusted = 0;
    neighboursAdjusted = false;
    projection = 0;
    contribMap = 0;
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
    delete contribMap;
    delete in;
    delete []bleach_count;
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

void FrameStack::setPanelBias(unsigned int wi, float scale, short bias){
  cout << "FrameStack::setPanelBias : " << wi << " (wave_no=" << wave_no << ")  " << scale << "\t" << bias << endl;
  if((int)wi >= wave_no)
    return;
  if(panelBiasMap.count(wi)){
    panelBiasMap[wi]->scale = scale;
    panelBiasMap[wi]->bias = bias;
    return;
  }
  panel_bias* pb = new panel_bias(scale, bias);
  for(uint i=0; i < sections.size(); ++i)
    sections[i]->setPanelBias(pb, wi);
  panelBiasMap.insert(make_pair(wi, pb));
}

void FrameStack::setBackgroundPars(unsigned int wi, int xm, int ym, float qnt, bool bg_subtract){
  if((int)wi >= wave_no)
    return;
  for(uint i=0; i < sections.size(); ++i)
    sections[i]->setBackgroundPars(wi, xm, ym, qnt, bg_subtract);
}

bool FrameStack::setChannelOffsets(vector<ChannelOffset> offsets)
{
  if(offsets.size() != channelOffsets.size())
    return(false);
  channelOffsets = offsets;
  return(true);
}

void FrameStack::setLookupTables(map<fluorInfo, SLookup*>* luts)
{
  for(uint i=0; i < sections.size(); ++i)
    sections[i]->setLookupTables(luts);
}

bool FrameStack::addFrame(Frame* frame){
  //cout << "FrameStack::addFrame excite : " << frame->excitation() << "--> " << frame->emission()
  //    << "  z pos : " << frame->zPos() << endl;
  if(!frame->ok()){
    cerr << "FrameStack::addFrame frame reports itself as being not ok. bummer" << endl;
    return(false);
  }
  if(!width){
    // just insert the frame into this one and set the appropriate parameters..
    real_x = frame->xPos();
    real_y = frame->yPos();
    z_begin = z_end = frame->zPos();
    width = frame->sampleWidth();
    height = frame->sampleHeight();
    pWidth = frame->p_width();
    pHeight = frame->p_height();

    if(width && height){
      bleach_count = new unsigned int[ pWidth * pHeight ];
      memset((void*)bleach_count, 0, sizeof(unsigned int) * pWidth * pHeight);
    }
    
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
    if(frame->xPos() != real_x || frame->yPos() != real_y){
	cerr << "Frame appears to not belong to this framestack: " << real_x << " != " << frame->xPos() << " || " << real_y << " != " << frame->yPos() << endl;
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
    
    projection = frameInformation->projection;   // though we may not need to know the projection .. 
    return;
  }
  // first initialise the projection
  projection = new float*[fwaves.size()];
  // resize the contrast data ..
  contrasts.resize(fwaves.size());
  for(uint i=0; i < fwaves.size(); i++){
    projection[i] = make_mip_projection(i, maxLevel, contrasts[i]);
  }
  frameInformation = new FrameInfo(fwaves.size(), projection, real_x, real_y, pixelX, pixelY, pWidth, pHeight);
  
}

void FrameStack::printFrames(){
    map<float, FrameSet*>::iterator it;
    int counter = 0;
    for(it = sectionMap.begin(); it != sectionMap.end(); it++){
	cout << "\t" << counter++ << "\t" << (*it).second->z_pos() << "\t" << (*it).second->x_pos() << ", " << (*it).second->y_pos() 
	     << "  size : " << (*it).second->width() << ", " << (*it).second->height() << "\t" << (*it).second->num_colors() << endl;
    }
}

bool FrameStack::contains_pixel(int x, int y)
{
  return( (x >= pixelX) && (x < right()) && (y > pixelY) && (y < top()) );
}

bool FrameStack::bleachCount_g(int x, int y, unsigned int& count)
{
  if(!contains_pixel(x, y) || !bleach_count)
    return(false);
  count = bleach_count[ (y-pixelY) * pWidth + (x - pixelX) ];
  return(true);
}

float FrameStack::bleachCountsMap_f(float* map, int map_x, int map_y, int map_w, int map_h)
{
  if( map_x > right() || (map_x + map_w) < left() || map_y > top() || (map_y + map_h) < bottom() )
    return(0);

  if(!bleach_count)
    return(0);

  int x_beg = map_x < pixelX ? pixelX : map_x;
  int x_end = (map_x + map_w) > right() ? right() : (map_x + map_w);
  int y_beg = map_y < pixelY ? pixelY : map_y;
  int y_end = (map_y + map_h) > top() ? top() : (map_y + map_h);

  if( x_beg >= x_end || y_beg >= y_end )
    return(0);
  // have to visit every pixel since we will convert to float
  float maxCount = 0;
  for(int y = y_beg; y < y_end; ++y){
    int m_y = y - map_y;
    int l_y = y - pixelY;
    for(int x = x_beg; x < x_end; ++x){
      int m_x = x - map_x;
      int l_x = x - pixelX;
      map[ m_y * map_w + m_x ] = (float)bleach_count[ l_y * pWidth + l_x ];
      maxCount = maxCount > map[ m_y * map_w + m_x ] ? maxCount : map[ m_y * map_w + m_x ];
    }
  }
  return(maxCount);
}

void FrameStack::setContribMap(float* map){
  if(contribMap)
    delete contribMap;
  contribMap = map;
  for(uint i=0; i < sections.size(); ++i)
    sections[i]->setContribMap(contribMap);
}

/* The below function has been replaced by a function in the idMap itself
   The new function does a gradual merging of overlapping panels 
   The contribMap is simply set directly from idMap */
// // This function needs to take into consideration the rolloff of the
// // image. Unfortunately I've not found any record for it anywhere.
void FrameStack::setContribMap(IdMap* idmap, ulong id){
  if(!contribMap)
    contribMap = new float[pWidth * pHeight];
  float* map = contribMap;
  cout << "Setting contrib map for : " << pixelX << "," << pixelY << endl;
  for(int dy=rolloff; dy < pHeight-rolloff; ++dy){
    map = contribMap + dy * pWidth + rolloff;
    for(int dx=rolloff; dx < pWidth-rolloff; ++dx){
      *map = idmap->count(id, pixelX + dx, pixelY + dy) ? 1.0 / float(idmap->count(pixelX + dx, pixelY + dy)) : 0.0;
      ++map;
    }
  }
  cout << endl;
  for(uint i=0; i < sections.size(); ++i)
    sections[i]->setContribMap(contribMap);
}

bool FrameStack::setNeighbour(FrameStack* neibour, int pos, bool recip){
    bool olaps = false;
//    cout << "FrameStack::setNeighbour pos : " << pos << "\trecip : " << recip << endl;
    switch(pos){
	case 0:
	    // left position
	    leftNeighbour = neibour;
	    leftOverlap = neibour->x_pos() + neibour->imageWidth() - real_x;
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
	    topOverlap = real_y + height - neibour->y_pos();
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
	    rightOverlap = real_x + width - neibour->x_pos();
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
	    bottomOverlap = neibour->y_pos() + neibour->imageHeight() - real_y;
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

void FrameStack::setBorders(){
  // experimental settings :
  // int rolloff = 0; // set at the beginninig of this file.. 
  leftBorder = left() + rolloff;
  rightBorder = right() - rolloff;
  bottomBorder = bottom() + rolloff;
  topBorder = top() - rolloff;


  // leftBorder = leftNeighbour ? (leftNeighbour->right() + left()) / 2 : left();
  // rightBorder = rightNeighbour ? ((rightNeighbour->left() + right()) / 2) : right();
  // bottomBorder = bottomNeighbour ? ((bottomNeighbour->top() + bottom()) / 2) : bottom();
  // topBorder = topNeighbour ? ((topNeighbour->bottom() + top()) / 2) : top();
}

int FrameStack::nearestBorderGlobal(int x, int y){
  int dx = x - leftBorder < rightBorder - x ? x - leftBorder : rightBorder - x;
  int dy = y - bottomBorder < topBorder - y ? y - bottomBorder : topBorder - y;
  return( dx < dy ? dx : dy );
}

vector<overlap_data*> FrameStack::adjustNeighbourPositions(unsigned int secNo, unsigned int rolloff, int instep, int window, int px, int py){

    cout << "FrameStack::adjustNeighbourPositions wavelegnth " << endl;
    float minCorr = 0.95;
    vector<overlap_data*> overlaps;
  
    if(neighboursAdjusted){  // don't do more than once.. 
	return(overlaps);
    }
    neighboursAdjusted = true;

    if(window >= (instep-1) || window < 0){    // instep is given as the number of lines inside of the rolloff.. 
	window = instep -1 ;
    }
    if(instep < 0){
	cerr << "FrameStack::adjustNeighbourPositions instep is less than 0 : " << instep << endl;
	return overlaps;
    }

    ImageFunc iFunc;
    //int margin = 50;

    if(rightNeighbour && !rightNeighbour->adjustedNeighbours()){   // then we may change the position of the right neighbour.. 
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

	cout << "Adjusting right Neighbour" << endl;
	cout << "\tpixelX : " << pixelX << "\tpixelY " << pixelY << endl
	     << "\tleft() : " << left() << "\tright() " << right() << endl
	     << "\tright neighbour->left() : " << rightNeighbour->left() << endl
	     << "\tgx: " << gx << "  nlix : " << nlix << " tlix: " << tlix << endl
	     << "\twindow : " << window << endl;
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
	overlaps.push_back(new overlap_data(real_x, real_y, rightNeighbour->x_pos(), rightNeighbour->y_pos(), px, py, px + 1, py, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
	if(off.corr > minCorr){
	    rightNeighbour->adjustPosition(off.dx, off.dy, RIGHT, off.corr);
	    vector<overlap_data*> olaps = rightNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px+1, py);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}
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
	overlaps.push_back(new overlap_data(real_x, real_y, topNeighbour->x_pos(), topNeighbour->y_pos(), px, py, px, py+1, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
	if(off.corr > minCorr){
	    topNeighbour->adjustPosition(off.dx, off.dy, TOP, off.corr);
	    vector<overlap_data*> olaps = topNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px, py+1);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}
    }
    if(leftNeighbour && !leftNeighbour->adjustedNeighbours()){
	int gx = (left() + leftNeighbour->right())/2;
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
	overlaps.push_back(new overlap_data(real_x, real_y, leftNeighbour->x_pos(), leftNeighbour->y_pos(), px, py, px-1, py, areaW, areaH, off.dx, off.dy, thisArea, neighborArea, off));
	if(off.corr > minCorr){
	    leftNeighbour->adjustPosition(off.dx, off.dy, LEFT, off.corr);
	    vector<overlap_data*> olaps = leftNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px-1, py);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}
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
	overlaps.push_back(new overlap_data(real_x, real_y, bottomNeighbour->x_pos(), bottomNeighbour->y_pos(), 
					    px, py, px, py-1, areaW, areaH, off.dx, off.dy, thisArea, neighbourArea, off));
	if(off.corr > minCorr){
	    bottomNeighbour->adjustPosition(off.dx, off.dy, BOTTOM, off.corr);
	    vector<overlap_data*> olaps = bottomNeighbour->adjustNeighbourPositions(secNo, rolloff, instep, window, px, py-1);
	    overlaps.insert(overlaps.end(), olaps.begin(), olaps.end());
	}
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
    //for(int i=1; i < 2; ++i){
    for(int i=0; i < wave_no; ++i){
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

// at the moment, chinfo has to be the same size as channelOffsets, and if specified
// raw has to have the same number of channels as chinfo and so on. This is a bit
// stupid since the interface doesn't make this clear. Should do something about that, sometime.. 
bool FrameStack::readToRGB(float* dest, int xpos, int ypos, 
			   unsigned int dest_width, unsigned int dest_height, 
			   unsigned int slice_no, vector<channel_info> chinfo,
			   raw_data* raw){
  if(slice_no >= sections.size()){
    cerr << "FrameStack::readToRGB (int version) slice no is too large : " << slice_no << endl;
    return(false);
  }
  // a little sanity check. It seems that we've started relying on too many different potentially conflicting
  // data structs. 
  if(raw && (raw->channels != chinfo.size())){
    cerr << "FrameStack::readToRGB raw_data is specified, but does not have same number of channels as chinfo: " 
	 << raw->channels << " != " << chinfo.size() << endl;
    exit(1);
  }
  // if chinfo is not channel_offsets size, then we can make
  vector<ChannelOffset> offsets = channelOffsets;

  if(chinfo.size() != channelOffsets.size()){
    cerr << "FrameStack::readToRGB channelOffsets is a different size fo chinfo: "
	 << channelOffsets.size() << " != " << chinfo.size() << endl;
    vector<ChannelOffset> tempOff(chinfo.size()); // initialises to 0 .. 
    offsets = tempOff;
    //    exit(1);
  }
  int dest_x, source_x, dest_y, source_y;
  unsigned int subWidth, subHeight;
  bool foundOverlap = false;
  // check if it overlaps with the border positions..
  for(unsigned int i=0; i < chinfo.size(); ++i){
    unsigned int corrected_slice = slice_no + offsets[i].z();
    if(corrected_slice >= sections.size())
      continue;
    if(globalToLocal(xpos + offsets[i].x(), ypos + offsets[i].y(), dest_width, dest_height, dest_x, 
		     source_x, dest_y, source_y, subWidth, subHeight)){
      if(raw){
	if(sections[slice_no]->readToRGB(dest, source_x, source_y, subWidth, subHeight, 
					 dest_x, dest_y, dest_width, chinfo[i], raw->values[i] + raw->positions[i])){
	  foundOverlap = true;
	  raw->positions[i] += (subWidth * subHeight);
	}
      }else{
	if(sections[slice_no]->readToRGB(dest, source_x, source_y, subWidth, subHeight, 
					 dest_x, dest_y, dest_width, chinfo[i]))
	  foundOverlap = true;
      }
    }
  }
  return(foundOverlap);
}


bool FrameStack::mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
				float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, raw_data* raw){
  if(!projection){
    finalise(maxLevel);
  }
  
  int dest_x, source_x, dest_y, source_y;
  unsigned int subWidth, subHeight;

  bool foundOverlap = false;
  
  for(uint wi=0; wi < channelOffsets.size(); ++wi){
    if(!raw && !( colors[wi].r || colors[wi].g || colors[wi].b ))
      continue;
    if(!globalToLocal(xpos + channelOffsets[wi].x(), ypos + channelOffsets[wi].y(), 
		      dest_width, dest_height, dest_x, source_x, dest_y, source_y, subWidth, subHeight))
      continue;
    foundOverlap = true;
    //    if(globalToLocal(xpos, ypos, dest_width, dest_height, dest_x, source_x, dest_y, source_y, subWidth, subHeight)){
    float v;
    //	for(uint wi=0; wi < fwaves.size(); ++wi){
    //if(!raw && !( colors[wi].r || colors[wi].g || colors[wi].b ))
    //	    continue;
    for(uint hp=0; hp < subHeight; ++hp){
      float* dst = dest + ((dest_y + hp) * dest_width + dest_x) * 3;   // dest is an rgb triplet..
      float* src = projection[wi] + (source_y + hp) * pWidth + source_x;
      float* mod = contribMap + (source_y + hp) * pWidth + source_x;
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
	//v =  (bias[wi] + (*src) * scale[wi]);
	v = (*mod) * (bias[wi] + (*src) * scale[wi]);
	if(v > 0){
	  dst[0] += v * colors[wi].r;
	  dst[1] += v * colors[wi].g;
	  dst[2] += v * colors[wi].b;
	}
	++src;
	dst += 3;
	++mod;
      }
    }
  }
  return(foundOverlap);
}


bool FrameStack::readToFloatPro(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, 
				unsigned int iheight, unsigned int wave)
{
    if(!projection){
      cout << "no projection so calling finalise" << endl;
	finalise(maxIntensity);
    }

    if(xb + iwidth > pWidth || yb + iheight > pHeight){
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

// The below allows the projection to be copied to a subregion of the destination data
// similarly to the readToFloatProGlobal, but without using the globalToLocal function
// (which uses the left_border, etc functions) thus allowing the data to be read to the 
// edge of the image.. 
// Uses LOCAL coordinates
bool FrameStack::readProjectionData(float* dest, uint d_width, uint d_x, uint d_y,  
				    uint xb, uint yb, uint c_width, uint c_height, uint wave){
  c_width = (xb + c_width) > pWidth ? pWidth - xb : c_width; 
  c_height = (yb + c_height) > pHeight ? pHeight - yb : c_height;

  // but since unsigned ints, this can still cause trouble for us
  if(xb + c_width > pWidth || yb + c_height > pHeight || wave > wave_no){
    cerr << "FrameStack::readProjectionData illegal coordinates "
	 << d_width << " : " << d_x << "," << d_y << "  xb,yb : " << xb << "," << yb
	 << "  c_width,c_height : " << c_width << "," << c_height << endl;
    return(false);
  }
  float* src = projection[wave] + yb * pWidth + xb;
  for(uint y=0; y < c_height; ++y){
    memcpy( (void*)(dest + (y + d_y) * d_width + d_x),
	    (void*)( src + y * pWidth ),
	    sizeof(float) * c_width
	    );
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
    //    float* dst = dest;
    //bool ok = true;
//    cout << "FrameStack::readToFloatProGlobal requested : " << xb << " +-> " << iwidth << " , " << yb << " +-> " << iheight << "  giving " << source_x << " +-> " << subWidth << " , "
//	 << source_y << " +-> " << subHeight << endl;
    
    for(uint yp = 0; yp < subHeight; ++yp){
	float* src = projection[wave] + (source_y + yp) * pWidth + source_x;
	float* dst = dest + (dest_y + yp) * iwidth + dest_x;
	//	memcpy((void*)dst, (const void*)src, sizeof(float) * subWidth);
	float* mod = contribMap + (source_y + yp) * pWidth + source_x;
	for(uint xp=0; xp < subWidth; ++xp){
	  *dst += (*src) * (*mod);
	  ++src;
	  ++dst;
	  ++mod;
	}
    }
    return(true);
}

BorderInfo* FrameStack::borderInformation(){
  BorderInfo* info = new BorderInfo(real_x, real_y);
  int x, y, w, h;
  if(leftNeighbour){
    x = left();
    y = bottom();
    w = leftNeighbour->right() - left();
    h = pHeight;
    info->setArea( borderArea(leftNeighbour, x, y, w, h), LEFT);
  }
  if(rightNeighbour){
    x = rightNeighbour->left();
    y = bottom();
    w = right() - x;
    h = pHeight;
    info->setArea( borderArea(rightNeighbour, x, y, w, h), RIGHT );
  }
  if(bottomNeighbour){
    x = left();
    y = bottom();
    w = pWidth;
    h = bottomNeighbour->top() - bottom();
    info->setArea( borderArea(bottomNeighbour, x, y, w, h), BOTTOM);
  }
  if(topNeighbour){
    x = left();
    y = topNeighbour->bottom();
    w = pWidth;
    h = top() - topNeighbour->bottom();
    info->setArea( borderArea(topNeighbour, x, y, w, h), TOP);
  }
  return(info);
}

BorderArea* FrameStack::borderArea(POSITION pos)
{
  BorderArea* ba = 0;
  FrameStack* nbr = 0;
  int x, y, h, w;
  if(LEFT && leftNeighbour){
    x = left();
    y = bottom();
    w = leftNeighbour->right() - left();
    h = pHeight;
    nbr = leftNeighbour;
  }
  if(RIGHT && rightNeighbour){
    x = rightNeighbour->left();
    y = bottom();
    w = right() - x;
    h = pHeight;
    nbr = rightNeighbour;
  }
  if(BOTTOM && bottomNeighbour){
    x = left();
    y = bottom();
    w = pWidth;
    h = bottomNeighbour->top() - bottom();
    nbr = bottomNeighbour;
  }
  if(TOP && topNeighbour){
    x = left();
    y = topNeighbour->bottom();
    w = pWidth;
    h = top() - topNeighbour->bottom();
    nbr = topNeighbour;
  }
  return( borderArea(nbr, x, y, w, h));
}

// x and y are in global coordinates. Converted within this function.
BorderArea* FrameStack::borderArea(FrameStack* nbor, int x, int y, int w, int h)
{
  // assume that this is correct.
  // The coordinates must work for this projection, but may nor work for
  // the neighbour and may need adjustment.
  
  int n_dx = x < nbor->left() ? nbor->left() - x : 0;
  int n_dy = y < nbor->bottom() ? nbor->bottom() - y : 0;
  int nw = w - n_dx;
  int nh = h - n_dy;

  float** t_data = new float*[wave_no];
  float** n_data = new float*[wave_no];
  for(uint i=0; i < wave_no; ++i){
    t_data[i] = new float[w * h];
    n_data[i] = new float[w * h];
    memset((void*)t_data[i], 0, sizeof(float) * w * h);
    memset((void*)n_data[i], 0, sizeof(float) * w * h);
    readToFloatPro( t_data[i], x - left(), w, y-bottom(), h, i);
    nbor->readProjectionData( n_data[i], w, n_dx, n_dy, 
			n_dx + x - nbor->left(), n_dy + y - nbor->bottom(),
			nw, nh, i );
    
  }
  unsigned int* tbleach = bleachCounts_g(x, y, w, h);
  unsigned int* nbleach = nbor->bleachCounts_g(x, y, w, h);
  BorderArea* ba = new BorderArea(t_data, n_data, wave_lengths, wave_no, tbleach, nbleach, x, y, w, h);
  return(ba);
}

const FrameStack* FrameStack::left_neighbour()
{
  return(leftNeighbour);
}

const FrameStack* FrameStack::right_neighbour()
{
  return(rightNeighbour);
}

const FrameStack* FrameStack::top_neighbour()
{
  return(topNeighbour);
}

const FrameStack* FrameStack::bottom_neighbour(){
  return(bottomNeighbour);
}

void FrameStack::clearBleachCount()
{
  if(!bleach_count)
    return;
  memset((void*)bleach_count, 0, sizeof(unsigned int) * pWidth * pHeight);
}

// x is defined as a class member,, buggers.. 
void FrameStack::incrementBleachCount(int nx, int ny, int radius, unsigned int count)
{
  // determine if there is any overlap
  if( !((nx + radius) > left() && (nx - radius) < right() && (ny + radius) > bottom() && (ny - radius) < top()) )
    return;
  // determine start and end positions
  int x_beg = (nx - radius) < left() ? left() : (nx - radius);
  int x_end = (nx + radius) > right() ? right() : (nx + radius);
  int y_beg = (ny - radius) < bottom() ? bottom() : (ny - radius);
  int y_end = (ny + radius) > top() ? top() : (ny + radius);
  // those limits might be a little unbalanced: maybe should be (x + radius + 1), but for now leave it
  int sq_radius = radius * radius;    // no need to call sqrt
  for(int y=y_beg; y < y_end; ++y){
    for(int x=x_beg; x < x_end; ++x){
      if( ((x-nx)*(x-nx) + (y-ny)*(y-ny)) < sq_radius )
	bleach_count[ (y-pixelY) * pWidth + (x-pixelX) ] += count;
    }
  }
}

// x and y in global coordinates 
unsigned int* FrameStack::bleachCounts_g(int x, int y, int w, int h)
{
  if(!w || !h || !bleach_count)
    return(0);
  unsigned int* binfo = new unsigned int[w * h];
  memset((void*)binfo, 0, sizeof(unsigned int)  * w * h);
  // if no overlap, then return immediately
  if(!bleach_count)
    return(binfo);
  
  if( !((x+w) > left() && x < right() && (y + h) > bottom() && y < top()) )
    return(binfo);
  int x_beg = x > left() ? x : left();
  int x_end = (x+w) < right() ? (x+w) : right();
  int y_beg = y > bottom() ? y : bottom();
  int y_end = (y+h) < top() ? (y+h) : top();
  int cp_width = x_end - x_beg;
  for(int gy=y_beg; gy < y_end; ++gy){
    memcpy( (void*)( binfo + (gy-y) * w + (x_beg-x) ), 
	    (void*)( bleach_count + (gy-pixelY) * w + (x_beg-pixelX) ), 
	    sizeof(unsigned int) * cp_width );
  }
  return(binfo);
}

float* FrameStack::make_mip_projection(unsigned int wi, float maxLevel, vector<float>& contrast){
  //unsigned int margin = 32;  // but we should be supplying this from somewhere else ? -ideally we should make it a property of the frameStack .. 
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
  return(proj);
}


bool FrameStack::readToFloat(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, unsigned int iheight, unsigned int secNo, unsigned int waveIndex, float maxLevel){
  //  cout << "FrameStackk::readToFloat" << endl;
    if(secNo >= sections.size()){
	cerr << "FrameStack::readToFloat secNo (" << secNo << ") is larger than sections size : " << sections.size() << endl;
	return(false);
    }
    return(sections[secNo]->readToFloat(dest, xb, yb, iwidth, iheight, 0, 0, iwidth, maxLevel, waveIndex));
}

bool FrameStack::readToFloat(float* dest, int xb, int iwidth, int yb, 
			     int iheight, int zb, int idepth,  unsigned int waveIndex, 
			     float maxLevel, bool use_cmap){   // simply read the appropriate pixels into a volume.. 
    // check that zb and idepth are ok.. zb must be > 0 and < sections.size..
    if(zb < 0 || zb >= int(sections.size()) || idepth <= 0 || zb + idepth > int(sections.size())){
	cerr << "FrameStack unsuitable z-sections requested : " << zb << "  -->  " << zb + idepth << endl;
	return(false);
    }
    
    if(waveIndex >= channelOffsets.size()){
      cerr << "FrameStack::readToFloat waveIndex is larger than channelOffsets : " << endl;
      return(false);
    }
    int dest_x, source_x, dest_y, source_y;      // though these are supposed to be unsigned.. (I really should change everything everywhere to signed)
    unsigned int subWidth, subHeight;   // the local coordinates of an overlap for dest and source respectively..
    if(!globalToLocal(xb + channelOffsets[waveIndex].x(), yb + channelOffsets[waveIndex].y(), 
		      iwidth, iheight, dest_x, source_x, dest_y, source_y, subWidth, subHeight)){
//	cout << "*  " << topBorder;
	return(false);   // this just means that there is no overlap and there is nothing more for us to do here..
    }
    
    float* dst = dest;
    // then just go through the z sections and call readToFloat on each one..
    bool ok = true;
    for(int zp=zb; zp < zb + idepth; ++zp){
      if(!sections[zp]->readToFloat(dst, source_x, source_y, subWidth, subHeight, dest_x, dest_y, iwidth, maxIntensity, waveIndex, use_cmap)){
	ok = false;
	//	    cout << "-//\\-" << endl;
      }
      dst += iwidth * iheight;
    }
    return(ok);    
}

bool FrameStack::readToShort(unsigned short* dest,unsigned int xb, unsigned int iwidth, unsigned int yb, 
			     unsigned int iheight, unsigned int secNo, unsigned int waveIndex){

  if(secNo >= sections.size()){
    cerr << "FrameStack::readToShort secNo (" << secNo << ") is larger than sections size : " 
	 << sections.size() << endl;
    return(false);
  }
  if(waveIndex >= channelOffsets.size()){
    cerr << "FrameStack::readToShort waveIndex is larger than channelOffsets " << endl;
    return(false);
  }

  int dest_x, source_x, dest_y, source_y;      // though these are supposed to be unsigned.. 
  unsigned int subWidth, subHeight;   // the local coordinates of an overlap for dest and source respectively..
  if(!globalToLocal(xb + channelOffsets[waveIndex].x(), yb + channelOffsets[waveIndex].y(), 
		    iwidth, iheight, dest_x, source_x, dest_y, source_y, subWidth, subHeight)){
    return(false);   // this just means that there is no overlap and there is nothing more for us to do here..
  }
  return(sections[secNo]->readToShort(dest, (unsigned int)source_x, (unsigned int)source_y, 
				      subWidth, subHeight, 
				      (unsigned int)dest_x, (unsigned int)dest_y, iwidth, waveIndex));
}

bool FrameStack::readToShort(unsigned short* dest, unsigned int secNo, unsigned int waveIndex){
  if(secNo >= sections.size())
    return(false);
  return(sections[secNo]->readToShort(dest, 0, 0, pWidth, pHeight, 0, 0, pWidth, waveIndex));
}

stack_stats FrameStack::stackStats(int xb, int yb, int s_width, int s_height, unsigned int waveIndex){
  return(stackStats(xb, yb, 0, s_width, s_height, (int)sections.size(), waveIndex));
}

stack_stats FrameStack::stackStats(int xb, int yb, int zb, int s_width, int s_height, int s_depth, unsigned int waveIndex){
  stack_stats stats;
  if(xb < 0 || yb < 0 || s_width <= 0 || s_height <= 0 || s_depth <= 0){
    cerr << "FrameStack::stackStats some negative dimension" << endl;
    return(stats);
  }
  if(xb + s_width > (int)pWidth || yb + s_height > (int)pHeight || (unsigned int)zb + s_depth > sections.size()){
    cerr << "FrameStack::stackStats some coordinates requested outside of frameStack" << endl;
    return(stats);
  }
  unsigned short* s_buffer = new unsigned short[s_width * s_height];
  vector<unsigned short> all_shorts;
  double sum = 0;
  all_shorts.reserve(s_width * s_height * s_depth);
  for(uint i=zb; i < zb + s_depth; ++i){
    memset((void*)s_buffer, 0, sizeof(unsigned short) * s_width * s_height);
    if(!sections[i]->readToShort(s_buffer, (unsigned int)xb, (unsigned int)yb, 
				 (unsigned int)s_width, (unsigned int)s_height, 0, 0, 
				 (unsigned int)s_width, waveIndex)){
      // delete stuff and return empty..
      cerr << "FrameStack::stackStats Unable to read short from frame" << endl;
      delete []s_buffer;
      return(stats);
    }
    for(int j=0; j < (s_width * s_height); ++j){
	all_shorts.push_back(s_buffer[j]);
	sum += (double)s_buffer[j];
    }
  }
  stats.mean = (unsigned short)(sum / (double)all_shorts.size());
  sort(all_shorts.begin(), all_shorts.end());
  stats.median = all_shorts[ all_shorts.size() / 2 ];
  // make 20 quantiles for the thingy..
  int qnt_no = 20;
  for(int i=0; i < qnt_no; ++i){
    stats.levels.push_back( float(i)/(float)qnt_no);
    stats.qntiles.push_back( all_shorts[ (i * all_shorts.size()) / qnt_no ] );
  }
  stats.minimum = all_shorts[0];
  stats.maximum = all_shorts.back();
  // and get the mode average as well.
  // use a vector of length 1000 to approximate.
  unsigned int d_length = 1000;
  vector<unsigned int> counts(d_length, 0);
  // use of a rounding function would probably give a better estimate
  // but it's not very likely to affect the mode for our distributions.. 
  for(unsigned int i=0; i < all_shorts.size(); ++i)
    counts[ ((d_length-1) * (all_shorts[i] - stats.minimum)) / (stats.maximum - stats.minimum) ]++;

  unsigned int max_count = 0;
  stats.mode = 0;
  for(unsigned int i=0; i < counts.size(); ++i){
    if(counts[i] > max_count){
      max_count = counts[i];
      stats.mode = ((stats.maximum - stats.minimum) * i) / counts.size();
    }
  }
  delete []s_buffer;
  return(stats);
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

void FrameStack::adjustPosition(QPoint p){
    // if I have been adjusted then this takes precedence, but 
    // probably we should change this function.. 
  pixelX += p.x();
  pixelY += p.y();
  if(frameInformation){
    frameInformation->xp = pixelX;
    frameInformation->yp = pixelY;
  }
  
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

    // consider also setting the left, right, top and bottom borders where appropriate.
    // after having done things to neighbours.
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

bool FrameStack::globalToLocal(int xpos, int ypos, int dest_width, int dest_height, 
			       int& dest_x, int& source_x, int& dest_y, int& source_y, 
			       unsigned int& subWidth, unsigned int& subHeight){
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
	// and then work out the width and height that we'll be taking out of this.. 
	subWidth = rightBorder <= xpos + int(dest_width) ? (rightBorder - pixelX) - source_x : (dest_width - dest_x);
	subHeight = topBorder <= ypos + int(dest_height) ? (topBorder - pixelY) - source_y : (dest_height - dest_y);
	return(true);
    }
    return(false);
}
