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

#ifndef FRAMESET_H
#define FRAMESET_H

#include "frame.h"
#include "../dataStructs.h"
#include <map>
#include <set>

class FrameSet {
    public :
	FrameSet(int* waveLengths, int waveNo);
    ~FrameSet();

    // addFrame creates an additional ifstream for each Frame
    bool addFrame(const char* fname, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
		  short numInt, short numFloat, unsigned short byteSize,
		  bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz);
    bool addFrame(Frame* frame);  // deletes frame if not good... 
    // since the frame constructor does the actual parsing of the file, create the frame first, then check if it 

    // we also need a whole load of accessor functions, and wrappers for the frame functions.. 
    float z_pos(){
	return(z);
    }
    float x_pos(){
	return(x);
    }
    float y_pos(){
	return(y);
    }
    float width(){
	return(w);
    }
    float height(){
	return(h);
    }
    int num_colors(){
	return(frames.size());
    }
//    bool hasWavelength(float wl){
//	return(frames.count(wl));
//    }

    bool readToRGB(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height,
		   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, std::vector<float> bias, 
		   std::vector<float> scale, std::vector<color_map> colors, 
		   bool bg_sub, raw_data* raw=0);
//    bool readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height,
//		   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, float waveLength);
 
    bool readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height,
		   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
		     float maxLevel, unsigned int waveIndex);
    //    bool readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height,
    //		   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
    //		     bool bg_sub, float maxLevel, unsigned int waveIndex);
   

    private :
	std::map<fluorInfo, Frame*> frames;    // we have one for each wavelength..
    std::vector<fluorInfo> fInfo;              // 
    // We also want to know what our position is .. ? 
//    std::set<fluorInfo> waves;             // check if we are supposed to know about the given wavelength.. 
    unsigned int pWidth, pHeight;       // the pixel height..
    float x, y, z, w, h;                // the physical position and size of the frameSet (obtain these from the first frame).
};

#endif
