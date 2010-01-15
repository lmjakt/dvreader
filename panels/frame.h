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

#ifndef FRAME_H
#define FRAME_H

#include <fstream>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "../dataStructs.h"

class Background;

typedef unsigned int uint;

// a small background struct. Only to be used by the Frame Class. Not to be passed around.

// background can do with a pos struct (area_pos = a_pos)
struct a_pos {
  int x, y;
  a_pos(){
    x = y = 0;
  }
  a_pos(int X, int Y){
    x = X; y = Y;
  }
};

struct td_bg {
  int x_m, y_m;
  int w, h;          // not using unsigned ints, since I get into so much trouble with such.
  float quantile;
  float* background;  // use float even if source is short.
  a_pos* bg_pos;
  float bg(int x, int y);
  td_bg(){
    x_m = y_m = w = h = 0;
    background = 0;
    bg_pos = 0;
  }
  ~td_bg(){
    delete background;
    delete bg_pos;
  }
};

class Frame {
    public :
	Frame(std::ifstream* inStream, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize, 
	      short numInt, short numFloat, unsigned short byteSize, 
	      bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz);
    
    ~Frame();

    void setStream(std::ifstream* inStream){
	in = inStream;
    }
   
    // simple accesor functions..
    bool ok();
    float emission();
    float excitation();
    float xPos();
    float yPos();
    float zPos();
    float sampleWidth();
    float sampleHeight();
    float exposure();
    float phSensor(){
      return(photoSensor);
    }
    void setPhSensorStandard(float phs){
      photoSensorStandard = phs;
      phSensor_m = photoSensorStandard / photoSensor;
    }
    unsigned int p_width(){
	return(pWidth);
    }
    unsigned int p_height(){
	return(pHeight);
    }

    // and functions to read data from the file.
    // These are horrible functions as they take far too many arguments that define how to read the data in
    // I should replace these with a pointer to some sort of data structure that is owned by the DeltaViewer
    // object, and which manipulates this when anything needs to be done. But maybe that can wait a bit.
    
    // the destination is a rectangular array, of width x height. fill data from source into the appropriate positions.. 
    bool readToRGB(float* dest, unsigned int source_x, unsigned int source_y, 
		   unsigned int width, unsigned int height, 
		   unsigned int dest_x, unsigned int dest_y, 
		   unsigned int dest_w, channel_info chinfo, 
		   float* raw=0);
    //		   float maxLevel, 
    //		   float bias, float scale, float r, float g, float b, bool bg_sub, float* raw=0); 


    //    bool readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
    //		     unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  
    //		     bool bg_sub, float maxLevel); 
    bool readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		     unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  
		     float maxLevel); 
    // NOTE the constructor will make a number of assumptions about the structure of the extended header. If these are not met, then 
    // it will set some flag to indicate failure.. 
    void setBackground(Background* bg, int z_pos);

    private :
	
    unsigned int pWidth;
    unsigned int pHeight;     // pixel parameters
    
    std::ifstream* in;        // the file from which we will be reading
    std::ios::pos_type frameOffset;     // the start position in the file from which we read
    unsigned short bno;  // the number of bytes for each value
    bool isReal;         // if true then count values as floats or doubles (note that bno=2 and isReal=true shouldn't be possible)
    bool isBigEndian;  // byte order .. (assume small endian unless otherwise noted)

    // the positions of the image in um.
    float x, y, z;        // the position of the frame (read from the header).. 
    float w, h;           // width and height
    float vx, vy, vz;     // the distances represented by one voxel.. (can be derived from the above number (except for vd)
    int zp;               // the z position, or rather the slice number
    // other image parameters...
    float photoSensor;
    float photoSensorStandard;  // the standard value
    float phSensor_m;           // photoSensor multiplier
    float timeStamp;     // starts counting from 0, in seconds
    float exposureTime;  // in seconds
    float excitationWavelength;
    float emissionWavelength;
    // there is more information in the extended header, but I'm not sure what it denotes.
    // A flag to indicate if everything ok..
    bool isOk;

    // td_background is a two dimensional background object,, let's try using
    // a three dimensional object Background instead..
    // a background object
    td_bg background; 
    Background* threeDBackground;
    channel_info channelInfo; // set by the readToRGB function.

    // assume float information..
    bool readToRGB_r(float* dest, unsigned int source_x, unsigned int source_y, 
		     unsigned int width, unsigned int height, 
		     unsigned int dest_x, unsigned int dest_y, 
		     unsigned int dest_w, channel_info chinfo,
		     float* raw=0);
    //		     float maxLevel, 
    //		     float bias, float scale, float r, float g, float b, float* raw=0); 
    // assume short information in file 
    bool readToRGB_s(float* dest, unsigned int source_x, unsigned int source_y, 
		     unsigned int width, unsigned int height, 
		     unsigned int dest_x, unsigned int dest_y, 
		     unsigned int dest_w, channel_info chinfo,
		     float* raw=0);
    //		     float maxLevel, 
    //		     float bias, float scale, float r, float g, float b, bool bg_sub, float* raw=0);

    //    bool readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
    //	       unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  
    //	       bool bg_sub, float maxLevel); 
    bool readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		       unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  
		       float maxLevel); 


    void swapBytes(char* data, unsigned int wn, unsigned int ws);

    bool setBackground(int xm, int ym, float qntile);   // xm, ym and qntile need to be settable.

    // functions that can be used to convert the raw 2 byte numbers to the relevant numbers.
    float convert_s(unsigned short* source, float bg, float bias, float scale, 
		    float maxLevel, float* raw, unsigned int xp, unsigned int yp){
      return( bias + scale * ((float)*source - bg) / maxLevel );
    }
    float convert_s_raw(unsigned short* source, float bg, float bias, float scale, 
			float maxLevel, float* raw, unsigned int xp, unsigned int yp){
      *raw = (float(*source) - bg) / maxLevel;
      return( bias + scale * ((float)*source - bg) / maxLevel );
    }
    float convert_s_contrast(unsigned short* source, float bg, float bias, float scale, 
			     float maxLevel, float* raw, unsigned int xp, unsigned int yp){
      source = xp > 0 ? source : source + 1;
      source = yp > 0 ? source : source + pWidth; // hack that causes incorrect behaviour at edges.
      int ct1 = abs( *(source - 1) - (*source) );
      int ct2 = abs( *(source - pWidth) - (*source) );
      int ct3 = abs( *(source - pWidth - 1) - (*source) );
      ct1 = ct1 > ct2 ? ct1 : ct2;
      ct1 = ct1 > ct3 ? ct1 : ct3;
      if(raw)
	(*raw) = float(ct1) / maxLevel;
      return(bias + scale * float(ct1)/maxLevel);
    }
    
    float to_float(unsigned short* source, float bg, 
		   float maxLevel, unsigned int xp, unsigned int yp){
      return( (float(*source) - bg) / maxLevel );
    }
    float to_float_contrast(unsigned short* source, float bg, 
			    float maxLevel, unsigned int xp, unsigned int yp){
      return( convert_s_contrast(source, bg, 0, 1.0, maxLevel, 0, xp, yp) );
    }
};
    
#endif
