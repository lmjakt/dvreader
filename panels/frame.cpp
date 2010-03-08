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
#include <sys/types.h>
#include <unistd.h>
#include "../image/background.h"

using namespace std;

// don't do any error checking as this function should only be used from within this 
// class. (Maybe I should declare it in the .cpp file rather than in the .h 

float td_bg::bg(int x, int y){
  int xb = (x - x_m/2) / x_m;
  int yb = (y - y_m/2) / y_m;

  // as long as x and y are not negative, then the smallest value we'll get here will
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

  float bot = background[pb] + ((float)(x - bg_pos[pb].x) / (float)x_m) * (background[pb+1] - background[pb]);
  float top = background[pb + bgw] + ((float)(x - bg_pos[pb+bgw].x) / (float)x_m) * (background[pb+bgw+1] - background[pb+bgw]);
  float b = bot + ((float)(y - bg_pos[pb].y)/(float)y_m) * (top - bot);
  return(b);
}

Frame::Frame(ifstream* inStream, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize, 
	     short numInt, short numFloat, unsigned short byteSize, 
	     bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz)
{
    threeDBackground = 0;
    zp = 0; 
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

//    cout << "extended header information : " << endl;
//    cout << "numInt : " << numInt << "  numFloat : " << numFloat << endl;
//    cout << "ints first: " << endl;
//    for(int i=0; i < numInt; ++i)
//      cout << i << " : " << headerInt[i] << endl;
//    cout << "and then the floats" << endl;
//    for(int i=0; i < numFloat; ++i)
//      cout << i << " : " << headerFloat[i] << endl;


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

    isOk = true;

    // DIC images set the excitation and emisson to -50 and -50. So doesn't make so much sense.
//     if(excitationWavelength > 300 && excitationWavelength < 1000 && emissionWavelength > 300 && emissionWavelength < 1300){ // stupid check, but..
// 	isOk = true;
//     }else{
//       cerr << "Strange header information obtained : excite " << excitationWavelength << "  emit " << emissionWavelength << endl;
//     }
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

bool Frame::readToRGB(float* dest, unsigned int source_x, unsigned int source_y, 
		      unsigned int width, unsigned int height, 
		      unsigned int dest_x, unsigned int dest_y, 
		      unsigned int dest_w, channel_info chinfo,
		      float* raw)
//		      float maxLevel, 
//		      float bias, float scale, float r, float g, float b, bool bg_sub, float* raw){
{
  // note that the dest has to be already initialised
  // we are only going to add to it..
  
  if(width > pWidth || height > pHeight || source_y >= pHeight || source_x >= pWidth){
    cerr << "Frame::readToRGB inappropriate coordinates : " << source_x << "\t" << source_y << "\t" << width << "\t" << height << endl;
    return(false);
  }
  channelInfo = chinfo;
  // at this point call the appropriate function to read ..
  if(isReal && bno == 4){
    return(readToRGB_r(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, chinfo, raw));
    //maxLevel, bias, scale, r, g, b, raw));
  }
  if(!isReal && bno == 2){
    return(readToRGB_s(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w,  chinfo, raw));
    //		       maxLevel, bias, scale, r, g, b, bg_sub, raw));
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


bool Frame::readToRGB_r(float* dest, unsigned int source_x, unsigned int source_y, 
			unsigned int width, unsigned int height, 
			unsigned int dest_x, unsigned int dest_y, 
			unsigned int dest_w, channel_info chinfo,
			float* raw){
  //			float maxLevel, float bias, float scale, float r, float g, float b, float* raw){
  cerr << "Frame::readToRGB unsupported data file type" << endl;
  return(false);
}

bool Frame::readToRGB_s(float* dest, unsigned int source_x, unsigned int source_y, 
			unsigned int width, unsigned int height, 
			unsigned int dest_x, unsigned int dest_y, 
			unsigned int dest_w, channel_info chinfo,
			float* raw){
  //			float maxLevel, 
  //			float bias, float scale, float r, float g, float b, bool bg_sub, float* raw){
    // 1. First make an appropriately sized buffer
    // 2. Seek to the appropriate position, and read into the buffer (necessary to do full width, but the height can ofcourse be done separately)
    // 3. Go through all values in the read position, do the transformation and 

  // The commented section refers to the use of a two dimensional background subtraction
  if(chinfo.bg_subtract && !background.x_m){
       cout << "Frame::readToRGB_s background subtraction requested: creating background object" << endl;
       channelInfo.bg_subtract = false;
       setBackground(16, 16, 0.2);
       channelInfo.bg_subtract = true;
  }

  // if(chinfo.bg_subtract && !threeDBackground){
  //   cerr << "Frame::readToRGB_s background subtraction requested, but no background object" << endl;
  //   exit(1);
  // }

  cout << "Frame::readToRGB_s wave: " << excitationWavelength << "  photoS : " << photoSensor << "  photoS_m " << phSensor_m << endl;
  unsigned short* buffer = new unsigned short[pWidth * height];   // which has to be 
  std::ios::pos_type startPos = frameOffset + (std::ios::pos_type)(pWidth * 2 * source_y);
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


  // Use a function pointer to distinguish the different variants of functions
  
  float (Frame::*convertFunction)(unsigned short*, float, float, float, float, float*, unsigned int, unsigned int) = 0;
  convertFunction = raw ? &Frame::convert_s_raw : &Frame::convert_s;
  convertFunction = chinfo.contrast ? &Frame::convert_s_contrast : convertFunction;
  
  for(unsigned int yp = 0; yp < height; yp++){
    source = buffer + yp * pWidth + source_x;
    dst = dest + (dest_y * dest_w + yp * dest_w + dest_x) * 3 ; // then increment the counters appopriately.. 
    for(unsigned int xp = 0; xp < width; xp++){
      //bg = chinfo.bg_subtract ? chinfo.maxLevel * threeDBackground->bg(xp, yp, zp) : 0;
      bg = chinfo.bg_subtract ? chinfo.maxLevel * background.bg(xp, yp) : 0;

      // variants on how to use the data provided
      //	*raw = (float(*source) - bg)/maxLevel;
      //v = bias + scale * phSensor_m * (float)(*source) / maxLevel;
      //v = bias + scale * (float)(*source) / (maxLevel * phSensorm);
      
      v = (*this.*convertFunction)(source, bg, chinfo.bias, chinfo.scale, chinfo.maxLevel, raw, xp, yp);

      //      v = v * phSensor_m;
 
     //v = bias + scale * ((float)*source - bg) / maxLevel;
      
      //	v = bias + scale * ((float)*source - (bg * maxLevel) )/maxLevel;
      //		v = bias + scale * (phSensor_m * float(*source) - (bg * maxLevel) )/maxLevel;
      //uv = bias + scale * (float(*source) / phSensor_m - (bg * maxLevel) )/maxLevel;
      if(raw)
	++raw;
      if(v > 0){
	dst[0] += v * chinfo.color.r;
	dst[1] += v * chinfo.color.g;
	dst[2] += v * chinfo.color.b;
      }
      dst += 3;
      ++source;
    }
  }
  delete buffer;
  // at which point we seem to have done everything required..
  return(true);
}


//bool Frame::readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
//		  unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
//		  bool bg_sub, float maxLevel){

bool Frame::readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, 
			  unsigned int width, unsigned int height, 
			  unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
			  float maxLevel){
  // 1. First make an appropriately sized buffer
  // 2. Seek to the appropriate position, and read into the buffer (necessary to do full width, but the height can ofcourse be done separately)
  // 3. Go through all values in the read position, do the transformation and 
  
  if(channelInfo.bg_subtract && !background.x_m){
    //  if(channelInfo.bg_subtract && !threeDBackground){
    cerr << "Frame::readToRGB_s background subtraction requested, but no background object" << endl;
    exit(1);
  }
  
  // The contrast function can't handle a single X-line (which is frequently asked for)
  // The below is a hack for this that does create incorrect behaviour (in that it shifts)
  // the values by one vertical position. It works, since the Frame::to_float_contrast function
  // ends up incrementing the starting position by 1 line if the relative y position is 0.
  // This is not a good answer, but the best that I can think of which won't take rather too
  // much time to implement. However, do be careful.
  unsigned int buffer_height = height;
  if(channelInfo.contrast && height == 1)
    buffer_height = 2;
  if(channelInfo.contrast && height == 1 && source_y == (pHeight - 1) )
    source_y--;

  unsigned short* buffer = new unsigned short[pWidth * buffer_height];   // which has to be 
  std::ios::pos_type startPos = frameOffset + (std::ios::pos_type)(pWidth * 2 * source_y);
  in->seekg(startPos);
  in->read((char*)buffer, pWidth * buffer_height * 2);
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
  
  float (Frame::*convertFunction)(unsigned short*, float, float, unsigned int, unsigned int) = 0;
  convertFunction = channelInfo.contrast ? &Frame::to_float_contrast : &Frame::to_float;
  
  for(unsigned int yp = 0; yp < height; yp++){
    source = buffer + yp * pWidth + source_x;
    dst = dest + (dest_y * dest_w + yp * dest_w + dest_x); // then increment the counters appopriately.. 
    for(unsigned int xp = 0; xp < width; xp++){
      //bg = channelInfo.bg_subtract ? channelInfo.maxLevel * threeDBackground->bg(xp, yp, zp) : 0;
      bg = channelInfo.bg_subtract ? background.bg(xp, yp) : 0;

      //      *dst = float(*source) / maxLevel;
      //*dst = (float(*source) - bg)/maxLevel;
      *dst = (*this.*convertFunction)(source, bg, channelInfo.maxLevel, xp, yp);

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
	    rect.push_back(buffer[ (dy + by * ym) * pWidth + (dx + bx * xm)]);
	    //	    rect.push_back(phSensor_m * buffer[ (dy + by * ym) * pWidth + (dx + bx * xm)]);
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

void Frame::setBackground(Background* bg, int z_pos){
  threeDBackground = bg;
  zp = z_pos;
}
