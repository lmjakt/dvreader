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

#include "frame.h"
#include <iostream>

using namespace std;

Frame::Frame(ifstream* inStream, ssize_t framePos, ssize_t readPos, ssize_t extHeadSize, 
	     short numInt, short numFloat, unsigned short byteSize, 
	     bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz)
{
    in = inStream;
    frameOffset = framePos;
    bno = byteSize;
    isReal = real;
    isBigEndian = bigEnd;
    
    pWidth = width;
    pHeight = height;


    vx = dx;
    vy = dy;
    vz = dz;  // the voxel coordinates.. these should then correspond to the width and height.. 

    w = float(width) * dx;
    h = float(height) * dy;    // 

    isOk = false;  // set to true if we get to the end.. 

    // Then read the remaining values in from the haeder..
    // Assume that the header should have at least 12 floats in it.
    // If not then we don't know how to read it ... so we will have to retturn setting isOk to false
    if(numFloat < 13){
	cerr << "Frame::Frame error numFloat is too small don't know how to read : " << numFloat << endl;
	return;
    }
    
    int* headerInt = new int[numInt];   // don't use sizeof(int), as the size on this machine doesn't really matter
    float* headerFloat = new float[numFloat];

    in->seekg(readPos);
    in->read((char*)headerInt, numInt * 4);    // but this fails if int is of a different size.
    in->read((char*)headerFloat, numFloat * 4); // and again this is dependant on various things..
    if(in->fail()){
	cerr << "Frame::Frame failed to read extended header" << endl;
	delete headerInt;
	delete headerFloat;
	return;
    }
    

    photoSensor = headerFloat[0];
    timeStamp = headerFloat[1];
    x = headerFloat[2] * -1.0;
    y = headerFloat[3] * -1.0;
    z = headerFloat[4];
    exposureTime = headerFloat[8];
    excitationWavelength = headerFloat[10];
    emissionWavelength = headerFloat[11];
    
    delete headerInt;
    delete headerFloat;
    if(excitationWavelength > 300 && excitationWavelength < 1000 && emissionWavelength > 300 && emissionWavelength < 1300){ // stupid check, but..
	isOk = true;
    }
}

Frame::~Frame(){
//    delete in;  // this isn't owned by frame anymore .. 
}

bool Frame::ok(){
    return(isOk);
}

float Frame::emission(){
    return(emissionWavelength);
}

float Frame::excitation(){
    return(excitationWavelength);
}

float Frame::xPos(){
    return(x);
}

float Frame::yPos(){
    return(y);
}

float Frame::zPos(){
    return(z);
}

float Frame::sampleWidth(){
    return(w);
}

float Frame::sampleHeight(){
    return(h);
}

float Frame::exposure(){
    return(exposureTime);
}

bool Frame::readToRGB(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		      unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, float bias, float scale, float r, float g, float b, float* raw){
    // note that the dest has to be already initialised
    // we are only going to add to it..
    
    if(width > pWidth || height > pHeight || source_y >= pHeight || source_x >= pWidth){
	cerr << "Frame::readToRGB inappropriate coordinates : " << source_x << "\t" << source_y << "\t" << width << "\t" << height << endl;
	return(false);
    }
    
    // at this point call the appropriate function to read ..
    if(isReal && bno == 4){
	return(readToRGB_r(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel, bias, scale, r, g, b, raw));
    }
    if(!isReal && bno == 2){
	return(readToRGB_s(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w,  maxLevel, bias, scale, r, g, b, raw));
    }
    cerr << "Frame::readToRGB unsupported data file type" << endl;
    return(false);
}

bool Frame::readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		      unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel){
    // note that the dest has to be already initialised
    // we are only going to add to it..
    
    if(width > pWidth || height > pHeight || source_y >= pHeight || source_x >= pWidth){
	cerr << "Frame::readToRGB inappropriate coordinates : " << source_x << "\t" << source_y << "\t" << width << "\t" << height << endl;
	return(false);
    }
    
    // at this point call the appropriate function to read ..
    if(isReal && bno == 4){
	cerr << "Frame::raedToFloat readToFloat_r not yet implemented " << endl;
	return(false);
	//return(readToFloat_r(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel));
    }
    if(!isReal && bno == 2){
	return(readToFloat_s(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w,  maxLevel));
    }
    cerr << "Frame::readToRGB unsupported data file type" << endl;
    return(false);
}


bool Frame::readToRGB_r(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
			unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, float bias, float scale, float r, float g, float b, float* raw){
    cerr << "Frame::readToRGB unsupported data file type" << endl;
    return(false);
}

bool Frame::readToRGB_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
			unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, float bias, float scale, float r, float g, float b, float* raw){
    // 1. First make an appropriately sized buffer
    // 2. Seek to the appropriate position, and read into the buffer (necessary to do full width, but the height can ofcourse be done separately)
    // 3. Go through all values in the read position, do the transformation and 
    
//    cout << "Frame::readToRGB_s source_x : " << source_x << "  source_y " << source_y << "  width " << width << "  height " << height
//	 << "  dest_x " << dest_x << "  dest_y " << dest_y << "  dest_w " << dest_w << "  maxLevel " << maxLevel << "  bias " << bias << "  scale  " << scale << endl;
    
    unsigned short* buffer = new unsigned short[pWidth * height];   // which has to be 
    ssize_t startPos = frameOffset + (pWidth * 2 * source_y);
    in->seekg(startPos);
    in->read((char*)buffer, pWidth * height * 2);
    if(in->fail()){
	delete buffer;
	cerr << "Frame::readToRGB_r unable to read from offset " << startPos << "\tfor " << pWidth * height * 2 << "\tbytes" << endl;
	in->clear();
	return(false);
    }
    if(isBigEndian){
	swapBytes((char*)buffer, pWidth * height, 2);
    }

    unsigned short* source;
    float* dst;
    if(raw){
	for(unsigned int yp = 0; yp < height; yp++){
//	    cout << "\treading line " << yp << endl;
	    source = buffer + yp * pWidth + source_x;
	    dst = dest + (dest_y * dest_w + yp * dest_w + dest_x) * 3 ; // then increment the counters appopriately.. 
	    for(unsigned int xp = 0; xp < width; xp++){
		// work out the appropriate position..
		// and perform the transformation..
		//float sv = float(*source);
		*raw = float(*source)/maxLevel;
		float v = bias + scale * (*raw);
//		float v = bias + scale * (float(*source)/maxLevel);
		++raw;
		if(v > 0){
		    dst[0] += v * r;
		    dst[1] += v * g;
		    dst[2] += v * b;
		}
		dst += 3;
		++source;
	    }
	}
    }else{
	for(unsigned int yp = 0; yp < height; yp++){
	    source = buffer + yp * pWidth + source_x;
	    dst = dest + (dest_y * dest_w + yp * dest_w + dest_x) * 3 ; // then increment the counters appopriately.. 
	    for(unsigned int xp = 0; xp < width; xp++){
		// work out the appropriate position..
		// and perform the transformation..
		//float sv = float(*source);
		float v = bias + scale * (float(*source)/maxLevel);
		if(v > 0){
		    dst[0] += v * r;
		    dst[1] += v * g;
		    dst[2] += v * b;
		}
		dst += 3;
		++source;
	    }
	}
    }
    delete buffer;
    // at which point we seem to have done everything required..
    return(true);
}

bool Frame::readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
			  unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel){
    // 1. First make an appropriately sized buffer
    // 2. Seek to the appropriate position, and read into the buffer (necessary to do full width, but the height can ofcourse be done separately)
    // 3. Go through all values in the read position, do the transformation and 
    
//    cout << "\tFrame::readToFloat_s source_x : " << source_x << "  source_y " << source_y << "  width " << width << "  height " << height
//	 << "  dest_x " << dest_x << "  dest_y " << dest_y << "  dest_w " << dest_w << "  maxLevel " << maxLevel << endl;
    
    unsigned short* buffer = new unsigned short[pWidth * height];   // which has to be 
    ssize_t startPos = frameOffset + (pWidth * 2 * source_y);
    in->seekg(startPos);
    in->read((char*)buffer, pWidth * height * 2);
    if(in->fail()){
	delete buffer;
	cerr << "Frame::readToFloat_s unable to read from offset " << startPos << "\tfor " << pWidth * height * 2 << "\tbytes" << endl;
	in->clear();
	return(false);
    }
    if(isBigEndian){
	swapBytes((char*)buffer, pWidth * height, 2);
    }

    unsigned short* source;
    float* dst;
    
    for(unsigned int yp = 0; yp < height; yp++){
	source = buffer + yp * pWidth + source_x;
	dst = dest + (dest_y * dest_w + yp * dest_w + dest_x); // then increment the counters appopriately.. 
	for(unsigned int xp = 0; xp < width; xp++){
	    // work out the appropriate position..
	    // and perform the transformation..
	    //float sv = float(*source);

	    *dst = float(*source)/maxLevel;

	    ++dst;
	    ++source;
	}
    }
    
    delete buffer;
    // at which point we seem to have done everything required..
    return(true);
}

void Frame::swapBytes(char* data, unsigned int wn, unsigned int ws){    // swaps in place
    char* word = new char[ws];  // the word size.. 
    for(uint i=0; i < wn; i++){
	for(unsigned int j=0; j < ws; j++){
	    word[j] = data[(i+1) * ws - (j+1)];
	}
	for(unsigned int j=0; j < ws; j++){
	    data[i * ws + j] = word[j];
	}
    }
    delete word;
}
