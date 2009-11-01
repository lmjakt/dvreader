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

typedef unsigned int uint;

class Frame {
    public :
	Frame(std::ifstream* inStream, size_t framePos, size_t readPos, size_t extHeadSize, 
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
    unsigned int p_width(){
	return(pWidth);
    }
    unsigned int p_height(){
	return(pHeight);
    }

    // and functions to read data from the file.

    // the destination is a rectangular array, of width x height. fill data from source into the appropriate positions.. 
    bool readToRGB(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  float maxLevel, float bias, float scale, float r, float g, float b, float* raw=0); 


    bool readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  float maxLeve); 
    // NOTE the constructor will make a number of assumptions about the structure of the extended header. If these are not met, then 
    // it will set some flag to indicate failure.. 

    private :
	
	unsigned int pWidth;
    unsigned int pHeight;     // pixel parameters
    
    std::ifstream* in;        // the file from which we will be reading
    size_t frameOffset;     // the start position in the file from which we read
    unsigned short bno;  // the number of bytes for each value
    bool isReal;         // if true then count values as floats or doubles (note that bno=2 and isReal=true shouldn't be possible)
    bool isBigEndian;  // byte order .. (assume small endian unless otherwise noted)

    // the positions of the image in um.
    float x, y, z;        // the position of the frame (read from the header).. 
    float w, h;           // width and height
    float vx, vy, vz;     // the distances represented by one voxel.. (can be derived from the above number (except for vd)
    
    // other image parameters...
    float photoSensor;
    float timeStamp;     // starts counting from 0, in seconds
    float exposureTime;  // in seconds
    float excitationWavelength;
    float emissionWavelength;
    // there is more information in the extended header, but I'm not sure what it denotes.
    // A flag to indicate if everything ok..
    bool isOk;

    // assume float information..
    bool readToRGB_r(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		     unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  float maxLevel, float bias, float scale, float r, float g, float b, float* raw=0); 
    // assume short information in file 
    bool readToRGB_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		     unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, float bias, float scale, float r, float g, float b, float* raw=0);
    bool readToFloat_s(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height, 
		       unsigned int dest_x, unsigned int dest_y, unsigned int dest_w,  float maxLeve); 


    void swapBytes(char* data, unsigned int wn, unsigned int ws);

};
    
#endif
