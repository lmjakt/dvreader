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
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

// don't do any error checking as this function should only be used from within this 
// class. (Maybe I should declare it in the .cpp file rather than in the .h 

float td_bg::bg(int x, int y){
  int xb = (x - x_m/2) / x_m;
  int yb = (y - y_m/2) / y_m;
  //  cout << "td_bg::bg : " << x << "," << y << "  xb,yb: " << xb << "," << yb << "  x_m,y_m : " << x_m << "," << y_m << endl;
  // as long as x and y are not negatvie, then the smallest value we'll get here will
  // be -0.5, which will be rouned to 0. So this should be a safe way of finding the appropriate
  // points from which to interpolate.
  
  // xb and yb can be 0, but we need xb to be smaller than the the width and height of the background.
  // note that this is not error checking as these are allowed values, but for which we need to make
  // some compensation.
  int bgw = w / x_m;
  int bgh = h / y_m;
  xb = xb < bgw - 1 ? xb : bgw - 2;
  yb = yb < bgh - 1 ? yb : bgh - 2;
  int pb = yb * bgw + xb;

  //cout << "\t" << "xb,yb" << xb << "," << yb << "  pb: " << pb << endl;

  float bot = background[pb] + ((float)(x - bg_pos[pb].x) / (float)x_m) * (background[pb+1] - background[pb]);
  float top = background[pb + bgw] + ((float)(x - bg_pos[pb+bgw].x) / (float)x_m) * (background[pb+bgw+1] - background[pb+bgw]);
  float b = bot + ((float)(y - bg_pos[pb].y)/(float)y_m) * (top - bot);
  return(b);
}

Frame::Frame(ifstream* inStream, size_t framePos, size_t readPos, size_t extHeadSize, 
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
		      unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, 
		      float bias, float scale, float r, float g, float b, bool bg_sub, float* raw){
    // note that the dest has to be already initialised
    // we are only going to add to it..
    
  cout << "Frame readToRGB : excite : " << excitationWavelength << " : " << photoSensor << endl;

    if(width > pWidth || height > pHeight || source_y >= pHeight || source_x >= pWidth){
	cerr << "Frame::readToRGB inappropriate coordinates : " << source_x << "\t" << source_y << "\t" << width << "\t" << height << endl;
	return(false);
    }
    
    // at this point call the appropriate function to read ..
    if(isReal && bno == 4){
	return(readToRGB_r(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel, bias, scale, r, g, b, raw));
    }
    if(!isReal && bno == 2){
        return(readToRGB_s(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w,  maxLevel, bias, scale, r, g, b, bg_sub, raw));
    }
    cerr << "Frame::readToRGB unsupported data file type" << endl;
    return(false);
}

//bool Frame::readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
//		      unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
//			bool bg_sub, float maxLevel){
bool Frame::readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
			unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
			float maxLevel){
    // note that the dest has to be already initialised2
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
      return(readToFloat_s(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel));
      //      return(readToFloat_s(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, bg_sub, maxLevel));
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
			unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, 
			float bias, float scale, float r, float g, float b, bool bg_sub, float* raw){
    // 1. First make an appropriately sized buffer
    // 2. Seek to the appropriate position, and read into the buffer (necessary to do full width, but the height can ofcourse be done separately)
    // 3. Go through all values in the read position, do the transformation and 
    
//    cout << "Frame::readToRGB_s source_x : " << source_x << "  source_y " << source_y << "  width " << width << "  height " << height
//	 << "  dest_x " << dest_x << "  dest_y " << dest_y << "  dest_w " << dest_w << "  maxLevel " << maxLevel << "  bias " << bias << "  scale  " << scale << endl;
    
//  bg_sub = false;

   if(bg_sub && !background.x_m){
     cout << "Frame::readToRGB_s background subtraction requested: creating background object" << endl;
     setBackground(16, 16, 0.2);
   }

    unsigned short* buffer = new unsigned short[pWidth * height];   // which has to be 
    size_t startPos = (uint)frameOffset + (pWidth * 2 * source_y);
    cout << "size of size_t : " << sizeof(size_t) << " startPos : " << startPos 
	 << "  frameOffset : " << (uint)frameOffset << " : pwidth " << pWidth << "  source_y " << source_y << endl;
    in->seekg(startPos);
    in->read((char*)buffer, pWidth * height * 2);
    if(in->fail()){
	delete buffer;
	cerr << "Frame::readToRGB_s unable to read from offset " << startPos << "\tfor " << pWidth * height * 2 << "\tbytes" << endl;
	in->clear();
	return(false);
    }
    if(isBigEndian){
	swapBytes((char*)buffer, pWidth * height, 2);
    }

    unsigned short* source;
    float* dst;
    float bg;  // background estimate
    float v;  // the value we calculate.. 
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
		bg = bg_sub ? background.bg(xp, yp) : 0;
		v = bias + scale * (phSensor_m * float(*source) - bg )/maxLevel;
		//float v = bias + scale * (*raw);
		//float v = bias + scale * (float(*source)/maxLevel);
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
	        bg = bg_sub ? background.bg(xp, yp) : 0;
		v = bias + scale * ( phSensor_m * float(*source) - bg )/maxLevel;
		//		float v = bias + scale * (float(*source)/maxLevel);
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

//bool Frame::readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
//		  unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
//		  bool bg_sub, float maxLevel){
bool Frame::readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
			  unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
			  float maxLevel){
    // 1. First make an appropriately sized buffer
    // 2. Seek to the appropriate position, and read into the buffer (necessary to do full width, but the height can ofcourse be done separately)
    // 3. Go through all values in the read position, do the transformation and 
    
  //    cout << "\tFrame::readToFloat_s source_x : " << source_x << "  source_y " << source_y << "  width " << width << "  height " << height
  //	 << "  dest_x " << dest_x << "  dest_y " << dest_y << "  dest_w " << dest_w << "  maxLevel " << maxLevel << endl;
    
//   bg_sub = false;
//   if(bg_sub && !background.x_m){
//     cout << "Frame::readToFloat_s background subtraction requested: creating background object" << endl;
//     setBackground(16, 16, 0.2);
//   }

//   if(!bg_sub){
//     cout << "Frame::readToFloat_s without background subtraction" << endl;
//   }

    unsigned short* buffer = new unsigned short[pWidth * height];   // which has to be 
    size_t startPos = frameOffset + (pWidth * 2 * source_y);
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
    float bg = 0;
    for(unsigned int yp = 0; yp < height; yp++){
	source = buffer + yp * pWidth + source_x;
	dst = dest + (dest_y * dest_w + yp * dest_w + dest_x); // then increment the counters appopriately.. 
	for(unsigned int xp = 0; xp < width; xp++){
	    // work out the appropriate position..
	    // and perform the transformation..
	    //float sv = float(*source);
	    //bg = bg_sub ? background.bg(xp, yp) : 0;
	    //    cout << float(*source) << " - " << bg << " = " << float(*source) - bg << endl;
	    *dst = float(*source) / maxLevel;
	    //*dst = (float(*source) - bg)/maxLevel;
	    *dst = *dst < 0 ? 0 : *dst;
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

bool Frame::setBackground(int xm, int ym, float qntile){
  if( xm >= pWidth || 
      ym >= pHeight || 
      qntile < 0 || 
      qntile >= 1.0 )
    {
      cerr << "Frame::setBackground called with bad parameters : " << xm << ", " << ym << ", " << qntile << endl;
      return(false);
    }
    cout << "Setting background " << endl;
    //  a bit ugly, but we'll need a float buffer. Since we don't know what maxlevel might get called,
    // we use 1.
    float* buffer = new float[pWidth * pHeight];
    // The 'false' below is VERY important, otherwise we'll end up in an infinite loop. 
    //    if(! readToFloat(buffer, 0, 0, pWidth, pHeight, 0, 0, pWidth, false, 1.0) ){
    if(! readToFloat(buffer, 0, 0, pWidth, pHeight, 0, 0, pWidth, 1.0) ){
      cerr << "Frame::setBackground unable to readToFloat : " << endl;
      delete buffer;
      return(false);
    }
    delete background.background;  // this should be ok, even if it is 0 according to something I read
    background.x_m = xm; background.y_m = ym; background.w = pWidth; background.h = pHeight; background.quantile = qntile;
    int bg_size =  (pWidth / xm) * (pHeight / ym);
    int bw = pWidth / xm;
    int bh = pHeight / ym;
    background.background = new float[ bg_size ];
    memset((void*)background.background, 0, sizeof(float) * bg_size);
    background.bg_pos = new a_pos[ bg_size ];

    // we need to get the background from each cell separately.
    for(int by=0; by < bh; ++by){
      for(int bx=0; bx < bw; ++bx){
	vector<float> rect;
	rect.reserve(xm * ym);
	for(int dy=0; dy < ym && (dy + by * ym) < pHeight; ++dy){
	  for(int dx=0; dx < xm && (dx + bx * xm) < pWidth; ++dx){
	    rect.push_back(phSensor_m * buffer[ (dy + by * ym) * pWidth + (dx + bx * xm)]);
	  }
	}
	sort(rect.begin(), rect.end());
	background.background[ by * bw + bx ] = rect[uint( float(rect.size()) * qntile )];
	background.bg_pos[by * bw + bx].x = ((bx+1) * xm) - xm/2; 
	background.bg_pos[by * bw + bx].y = ((by+1) * ym) - ym/2;
      }
    }
    return(true); 
}
