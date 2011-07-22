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
#include "sLookup.h"
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <unistd.h>
#include "../image/background.h"

using namespace std;

Frame::Frame(ifstream* inStream, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize, 
	     short numInt, short numFloat, unsigned short byteSize, 
	     bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz)
{
  // we should try to remove all references to threeDBackground as we gave up on it previously
    threeDBackground = 0;
    lookup_table = 0;  //only use if present.. 
    // Instead use td_background (two dimensional specific to the frame)
    background = new Two_D_Background();
    background->setParameters(0.2, 16, 16);

    contribMap = 0;
    panelBias = 0;
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
      delete []headerInt;
      delete []headerFloat;
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
    
    delete []headerInt;
    delete []headerFloat;

    isOk = true;

    // DIC images set the excitation and emisson to -50 and -50. So doesn't make so much sense.
    // to check the values here
}

Frame::~Frame(){
//    delete in;  // this isn't owned by frame anymore .. 
  delete background;
}

void Frame::setLookupTable(SLookup* lut)
{
  lookup_table = lut;
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

void Frame::setBias(panel_bias* pb){
  panelBias = pb;
}

bool Frame::readToRGB(float* dest, unsigned int source_x, unsigned int source_y, 
		      unsigned int width, unsigned int height, 
		      unsigned int dest_x, unsigned int dest_y, 
		      unsigned int dest_w, channel_info chinfo,
		      float* raw)
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
    return(readToRGB_r(dest, source_x, source_y, width, height, 
		       dest_x, dest_y, dest_w, chinfo, raw));
  }
  if(!isReal && bno == 2){
    return(readToRGB_s(dest, source_x, source_y, width, height, 
		       dest_x, dest_y, dest_w,  chinfo, raw));
  }
  cerr << "Frame::readToRGB unsupported data file type" << endl;
  return(false);
}

bool Frame::readToFloat(float* dest, unsigned int source_x, unsigned int source_y, 
			unsigned int width, unsigned int height, 
			unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
			float maxLevel, bool use_cmap){
    // note that the dest has to be already initialised
    // we are only going to add to it..
    
    if(width > pWidth || height > pHeight || source_y >= pHeight || source_x >= pWidth){
	cerr << "Frame::readToRGB inappropriate coordinates : " << source_x << "\t" 
	     << source_y << "\t" << width << "\t" << height << endl;
	return(false);
    }
    
    // at this point call the appropriate function to read ..
    if(isReal && bno == 4){
	cerr << "Frame::raedToFloat readToFloat_r not yet implemented " << endl;
	return(false);
	//return(readToFloat_r(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel));
    }
    if(!isReal && bno == 2){
      return(readToFloat_s(dest, source_x, source_y, width, height, 
			   dest_x, dest_y, dest_w, maxLevel, use_cmap));
    }
    cerr << "Frame::readToRGB unsupported data file type" << endl;
    return(false);
}

bool Frame::readToShort(unsigned short* dest, unsigned int source_x, unsigned int source_y, 
			unsigned int width, unsigned int height,
			unsigned int dest_x, unsigned int dest_y, unsigned int dest_w){

  if(width > pWidth || height > pHeight || source_y >= pHeight || source_x >= pWidth){
    cerr << "Frame::readToRGB inappropriate coordinates : " << source_x << "\t" 
	 << source_y << "\t" << width << "\t" << height << endl;
    return(false);
  }
  if(bno != 2){
    cerr << "Frame::readToShort unable to read to short since raw format is not short" << endl;
    return(false);
  }
  //  unsigned short* buffer = new unsigned short[pWidth * height];
  // bool sepBuffer = true;
  unsigned short* buffer = dest + (dest_y * dest_w);
  bool sepBuffer = false;
  if(width != dest_w || width != pWidth){
    buffer = new unsigned short[pWidth * height];   // which has to be 
    sepBuffer = true;
  }
  std::ios::pos_type startPos = frameOffset + (std::ios::pos_type)(pWidth * 2 * source_y);
  in->seekg(startPos);
  in->read((char*)buffer, pWidth * height * 2);
  if(in->fail()){
    if(sepBuffer)
      delete []buffer;
    cerr << "Frame::readToShort unable to read from offset : "
	 << startPos << " for " << pWidth * height * 2 << " bytes" << endl;
    in->clear();
    return(false);
  }
  if(isBigEndian)
    swapBytes((char*)buffer, pWidth * height, 2);
  applyBiasToShort(buffer, pWidth * height);
  if(!sepBuffer)
    return(true);
  
  // if sepBuffer we need to copy from buffer to destination
  // this needs to be done line by line.. but we can use memcpy
  unsigned short* dst = dest + (dest_y * dest_w) + dest_x;
  unsigned short* source = buffer + source_x;
  for(unsigned int yp=0; yp < height; ++yp){
    memcpy((void*)dst, (const void*)source, width*2);
    dst += dest_w;
    source += pWidth;
  }
  delete []buffer;
  return(true);

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

  if(chinfo.bg_subtract && !background->backgroundSet()){
    // changing from true to false to true avoids an infinite loop or 
    // exit (as setBackground calls readToFloat which checks background).
    channelInfo.bg_subtract = false;
    setBackground();
    channelInfo.bg_subtract = true;
  }

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
  float* map;  // the contribution map


  // Use a function pointer to distinguish the different variants of functions
  
  float (Frame::*convertFunction)(unsigned short*, float, float, float, float, float, float*, unsigned int, unsigned int) = 0;
  if(!panelBias || !panelBias->biassed()){
    convertFunction = raw ? &Frame::convert_s_raw : &Frame::convert_s;
    convertFunction = chinfo.contrast ? &Frame::convert_s_contrast : convertFunction;
  }else{
    convertFunction = raw ? &Frame::convert_s_raw_pb : &Frame::convert_s_pb;
    convertFunction = chinfo.contrast ? &Frame::convert_s_contrast : convertFunction;
  }
  // if simple conversion use lookup table..
  if(convertFunction == &Frame::convert_s){
    cout << "Calling lookup_table->addToRGB : " << excitationWavelength << " --> " << emissionWavelength << endl;
    lookup_table->addToRGB_f(buffer, source_x, 0, pWidth,
			     dest, dest_x, dest_y, dest_w,
			     width, height, contribMap + source_y * pWidth); // we always read full lines.. 
    delete []buffer;
    return(true);
  }
  for(unsigned int yp = 0; yp < height; yp++){
    source = buffer + yp * pWidth + source_x;
    dst = dest + (dest_y * dest_w + yp * dest_w + dest_x) * 3 ; // then increment the counters appopriately.. 
    map = contribMap + (source_y + yp) * pWidth + source_x; 
    for(unsigned int xp = 0; xp < width; xp++){
      //bg = chinfo.bg_subtract ? chinfo.maxLevel * threeDBackground->bg(xp, yp, zp) : 0;
      bg = chinfo.bg_subtract ? background->bg(xp + source_x, yp + source_y) : 0;
      //      bg = chinfo.bg_subtract ? chinfo.maxLevel * background.bg(xp, yp) : 0;
      // variants on how to use the data provided
      //	*raw = (float(*source) - bg)/maxLevel;
      //v = bias + scale * phSensor_m * (float)(*source) / maxLevel;
      //v = bias + scale * (float)(*source) / (maxLevel * phSensorm);
      
      v = (*this.*convertFunction)(source, bg, chinfo.bias, chinfo.scale, chinfo.maxLevel, *map, raw, xp, yp);

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
      ++map;
    }
  }
  delete []buffer;
  // at which point we seem to have done everything required..
  return(true);
}


//bool Frame::readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
//		  unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
//		  bool bg_sub, float maxLevel){

bool Frame::readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, 
			  unsigned int width, unsigned int height, 
			  unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
			  float maxLevel, bool use_cmap){
  // 1. First make an appropriately sized buffer
  // 2. Seek to the appropriate position, and read into the buffer (necessary to do full width, but the height can ofcourse be done separately)
  // 3. Go through all values in the read position, do the transformation and 
  
  if(channelInfo.bg_subtract && !background->backgroundSet()){
    //  if(channelInfo.bg_subtract && !threeDBackground){
    channelInfo.bg_subtract=false;
    setBackground();
    channelInfo.bg_subtract=true;
    cerr << "Frame::readToFloat_s background subtraction requested, but no background object" << endl;
    //    exit(1);
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
  if(panelBias && panelBias->biassed()){
    convertFunction = &Frame::to_float_pb;
  }else{
    convertFunction = channelInfo.contrast ? &Frame::to_float_contrast : &Frame::to_float;
  }
  float* cmap;
  float value;
  for(unsigned int yp = 0; yp < height; yp++){
    source = buffer + yp * pWidth + source_x;
    dst = dest + (dest_y * dest_w + yp * dest_w + dest_x); // then increment the counters appopriately.. 
    cmap = contribMap + (yp + source_y) * pWidth + source_x;
    for(unsigned int xp = 0; xp < width; xp++){
      //bg = channelInfo.bg_subtract ? channelInfo.maxLevel * threeDBackground->bg(xp, yp, zp) : 0;
      bg = channelInfo.bg_subtract ? background->bg(xp + source_x, yp + source_y) : 0;

      //      *dst = float(*source) / maxLevel;
      //*dst = (float(*source) - bg)/maxLevel;
      //      *dst = (*this.*convertFunction)(source, bg, channelInfo.maxLevel, xp, yp);
      value =  (*this.*convertFunction)(source, bg, maxLevel, xp, yp);
      value = value < 0 ? 0 : value;
      if(!use_cmap){
	*dst = value;
      }else{
	*dst += (*cmap * value);
      }
      //*dst = *dst < 0 ? 0 : *dst;
      ++dst;
      ++source;
      ++cmap;
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

//bool Frame::setBackground(int xm, int ym, float qntile){
bool Frame::setBackground(){
  // x_m, y_m quantile are now set in setBackgroundPars and their values checked there
  // otherwise hard coded values are set in the Frame constructor.
  //  int xm = background.x_m;
  //int ym = background.y_m;

  // if( xm >= pWidth || 
  //     ym >= pHeight || 
  //     qntile < 0 || 
  //     qntile >= 1.0 )
  //   {
  //     cerr << "Frame::setBackground called with bad parameters : " << xm << ", " << ym << ", " << qntile << endl;
  //     return(false);
  //   }
  //  a bit ugly, but we'll need a float buffer. Since we don't know what maxlevel might get called,
  // we use 1.
  float* buffer = new float[pWidth * pHeight];
  if(! readToFloat(buffer, 0, 0, pWidth, pHeight, 0, 0, pWidth, 1.0) ){
    cerr << "Frame::setBackground unable to readToFloat : " << endl;
    delete buffer;
    return(false);
  }
  delete []buffer;
  if(!background->setBackground(pWidth, pHeight, buffer)){
    cerr << "unable to set background. we should probably die or something" << endl;
    return(false);
  }
  return(true);

  // delete background.background;  // this should be ok, even if it is 0 according to something I read
  // // the following are already set.. 
  // //    background.x_m = xm; background.y_m = ym; background.w = pWidth; background.h = pHeight; background.quantile = qntile;
  // int bg_size =  (pWidth / background.x_m) * (pHeight / background.y_m);
  // int bw = pWidth / background.x_m;
  // int bh = pHeight / background.y_m;
  // background.background = new float[ bg_size ];
  // memset((void*)background.background, 0, sizeof(float) * bg_size);
  // background.bg_pos = new a_pos[ bg_size ];
  
  // // we need to get the background from each cell separately.
  // for(int by=0; by < bh; ++by){
  //   for(int bx=0; bx < bw; ++bx){
  //     vector<float> rect;
  //     rect.reserve(xm * ym);
  //     for(int dy=0; dy < ym && (dy + by * ym) < (int)pHeight; ++dy){
  // 	for(int dx=0; dx < xm && (dx + bx * xm) < (int)pWidth; ++dx){
  // 	  rect.push_back(buffer[ (dy + by * ym) * pWidth + (dx + bx * xm)]);
  // 	  //	    rect.push_back(phSensor_m * buffer[ (dy + by * ym) * pWidth + (dx + bx * xm)]);
  // 	}
  //     }
  //     sort(rect.begin(), rect.end());
  //     background.background[ by * bw + bx ] = rect[uint( float(rect.size()) * background.quantile )];
  //     background.bg_pos[by * bw + bx].x = ((bx+1) * xm) - xm/2; 
  //     background.bg_pos[by * bw + bx].y = ((by+1) * ym) - ym/2;
  //   }
  // }
  // return(true); 
}

void Frame::setBackground(Background* bg, int z_pos){
  threeDBackground = bg;
  zp = z_pos;
}

void Frame::applyBiasToShort(unsigned short* data, int length){
  if(!panelBias || !panelBias->bias)
    return;
  short bias = panelBias->bias;
  unsigned short* end = data + length;
  for(unsigned short* ptr=data; ptr < end; ++ptr)
    (*ptr) += bias;
}

void Frame::setContribMap(float* map){
  contribMap = map;
}

void Frame::setBackgroundPars(int xm, int ym, float qnt, bool subtract_bg){
  // first check if the same..
  channelInfo.bg_subtract = subtract_bg;
  if(xm > 0 && xm < (int)pWidth / 2
     && ym > 0 && ym < (int)pHeight / 2
    && qnt >= 0 && qnt <= 1.0){  
    background->setParameters(qnt, xm, ym);
  }
  return;

  // if(background.x_m == xm && background.y_m == ym && background.quantile == qnt)
  //   return;
  
  // if(xm > 0 && xm < (int)pWidth / 2
  //    && ym > 0 && ym < (int)pHeight / 2
  //   && qnt >= 0 && qnt <= 1.0){  
  //   background.x_m = xm;
  //   background.y_m = ym;
  //   background.quantile = qnt;
  //   delete []background.background;
  //   background.background = 0;
  // }
}
