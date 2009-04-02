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

#include "colorMapper.h"
#include <iostream>

using namespace std;

ColorMapper::ColorMapper(){
    // set a few things to 0...
    mappingState = 0;    // in which case we never do anything when run..
    mergedOffsets = 0;
}

void ColorMapper::mapSingle(float* source, int W, int H, int XO, int YO, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, bool add){
    mappingState = 1;
    w = W;
    h = H;
    xo = XO;
    yo = YO;  // ok.. 
    init(source, w * h, dest, frameDest, frameInterval, color, bias, scale, add);
    start();
    //run();
}

void ColorMapper::mapMerged(float* source, int W, int H, int* XO, int* YO, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, int* mColors, int mSize, bool add){
    mappingState = 2;
    w = W;
    h = H;
    xof = XO;
    yof = YO;
    mergedColors = mColors;
    mergedSize = mSize;
    if(mergedOffsets){
	delete mergedOffsets;
    }
    mergedOffsets = new int[mergedSize];
    for(int i=0; i < mergedSize; i++){
	mergedOffsets[i] = mergedColors[i] * w * h;   // since pixNo is the size of 1 frame... hmm 
    }
    init(source, w * h, dest, frameDest, frameInterval, color, bias, scale, add);
    start();
    //run();
    //delete mergedOffsets;
}

void ColorMapper::mapMergedIndividualComponents(float* source, int W, int H, int* XO, int* YO, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, int* mColors, int mSize, bool add){
    mappingState = 3;
    w = W;
    h = H;
    xof = XO;
    yof = YO;
    mergedColors = mColors;    // this is needed for this one as we need to know which biasFactors and scaleFactors to use.. 
    mergedSize = mSize;
    if(mergedOffsets){
	delete mergedOffsets;
    }
    mergedOffsets = new int[mergedSize];
    for(int i=0; i < mergedSize; i++){
	mergedOffsets[i] = mergedColors[i] * w * h;   // since pixNo is the size of 1 frame... hmm 
    }
    
    init(source, w * h, dest, frameDest, frameInterval, color, bias, scale, add);
    start();
    //run();
//    delete mergedOffsets;
}

void ColorMapper::init(float* source, int pixNo, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, bool add){
    dataSource = source;
    pixelNo = pixNo;
    destData = dest;
    frameDestData = frameDest;
    frameIntervalNo = frameInterval;
    colors = color;
    biasFactors = bias;
    scaleFactors = scale;
    additive = add;
}

void ColorMapper::run(){
    switch(mappingState){
	case 0 :
	    cerr << "mapping State is 0 do nothing" << endl;
	    break;
	case 1:
	    map_single();
	    break;
	case 2:
	    map_merged();
	    break;
	case 3:
	    map_merged_individual_components();
	    break;
	default :
	    cerr << "Unknown mapping state : " << mappingState << endl;
    }
}

void ColorMapper::map_single(){
    // use colours and others to add the colours..
    float v;
    float sf = *scaleFactors;
    float bf = *biasFactors;
    xo = xo < 0 ? 0 : xo;
    yo = yo < 0 ? 0 : yo;  // make sure these are not negative...
    
    // note that we have to increment the dataSource, the destData, and the frameDestData by 
    // the appropriate number..
    int pixelSkip = yo * w + xo;
    dataSource += pixelSkip;
    frameDestData += pixelSkip * frameIntervalNo;
    // don't change the destination data, as we wan't to move the pixels.. 

    // and then we can start going through the things...
    for(int y=yo; y < h; y++){
	for(int x=xo; x < w; x++){
	    (*frameDestData) = (*dataSource);
	    v = (*dataSource) * sf + bf;
	    if(v > 0){
		if(!additive){ v = -v; }
		destData[0] += v * colors[0];
		destData[1] += v * colors[1];
		destData[2] += v * colors[2];
	    }
	    ++dataSource;
	    destData += 3;
	    frameDestData += frameIntervalNo;
	}
	destData += (3 * xo);
	dataSource += xo;
	frameDestData += (frameIntervalNo * xo);
    }
    
}

void ColorMapper::map_merged(){
    float v;
    float sf = *scaleFactors;
    float bf = *biasFactors;

    // first we have to work out the biggest offset, and use that as the starting point. 
    // as there is no point in using anything else...
    xo = yo = 0;
    cout << "map merged : " << endl;
    for(int i=0; i < mergedSize; i++){
	cout << "\tindex " << i << "  xof " << xof[mergedColors[i]] << "  yof " << yof[mergedColors[i]] << endl;
	xo = xo < xof[mergedColors[i]] ? xof[mergedColors[i]] : xo;
	yo = yo < yof[mergedColors[i]] ? yof[mergedColors[i]] : yo;
    }
//    dataSource += yo * w + xo;
    cout << "\txo --> " << xo << endl;
    for(int y=yo; y < h; y++){
	for(int x=xo; x < w; x++){
	    v = 1.0;
	    for(int i=0; i < mergedSize; i++){
		v *= dataSource[mergedOffsets[i] + yof[mergedColors[i]] * w + xof[mergedColors[i]]];
	    }
	    (*frameDestData) = v;
	    v = v * sf + bf;
	    if(v > 0){
		if(!additive){ v = -v; }
		destData[0] += v * colors[0];
		destData[1] += v * colors[1];
		destData[2] += v * colors[2];
	    }
	    ++dataSource;
	    destData += 3;
	    frameDestData += frameIntervalNo;
	}
	dataSource += xo;
	frameDestData += (frameIntervalNo * xo);
	destData += (3 * xo);
    }
}

void ColorMapper::map_merged_individual_components(){
    // unfortunately for this we still need to calculate v as the product of the indivudal things like before,
    // hmmm, this is actually debatable... perhaps one should think a bit about it..
    float v;
    float tv;
    xo = yo = 0;
    for(int i=0; i < mergedSize; i++){
	xo = xo < xof[mergedColors[i]] ? xof[mergedColors[i]] : xo;
	yo = yo < yof[mergedColors[i]] ? yof[mergedColors[i]] : yo;
    }
    for(int y=yo; y < h; y++){
	for(int x=xo; x < w; x++){
	    v = 1.0;
	    for(int i=0; i < mergedSize; i++){
		tv = dataSource[mergedOffsets[i] + yof[mergedColors[i]] * w + xof[mergedColors[i]]] * scaleFactors[mergedColors[i]] + biasFactors[mergedColors[i]];
		tv = tv < 0 ? 0 : tv;
		v *= tv;
	    }
	    (*frameDestData) = v;
	    if(v > 0){
		if(!additive){ v = -v; }
		destData[0] += v * colors[0];
		destData[1] += v * colors[1];
		destData[2] += v * colors[2];
	    }
	    ++dataSource;
	    destData += 3;
	    frameDestData += frameIntervalNo;
	}
	dataSource += xo;
	frameDestData += (frameIntervalNo * xo);
	destData += (3 * xo);
    }
    
//     for(int i=0; i < pixelNo; i++){
// 	v = 1;
// 	for(int j=0; j < mergedSize; j++){
// 	    tv = (dataSource[mergedOffsets[j]] * scaleFactors[mergedColors[j]] + biasFactors[mergedColors[j]]);
// 	    tv = tv < 0 ? 0 : tv;
// 	    v *= tv;
// 	}
// 	(*frameDestData) = v;
// 	if(v > 0){
// 	    destData[0] += v * colors[0];
// 	    destData[1] += v * colors[1];
// 	    destData[2] += v * colors[2];
// 	}
// 	++dataSource;
// 	destData += 3;
// 	frameDestData += frameIntervalNo;
//     }
}
