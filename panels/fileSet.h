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

#ifndef FILESET_H
#define FILESET_H

#include "frameStack.h"
#include "../dataStructs.h"
#include <map>
#include <vector>
#include <string>



class FileSet {
    public :
	FileSet(int* waveLengths, int waveNo, float maxLevel);
    ~FileSet();

    bool addFrame(std::string fname, std::ifstream* in, std::ios::pos_type framePos, 
		  std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
		  short numInt, short numFloat, unsigned short byteSize,
		  bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz);
    bool getStack(float& xpos, float& ypos);   // sets the appropriate values up for a given thingy.. 
    bool finalise();   // checks for a complete rectangle and sets up the x, y, and z_position vectors

    bool readToRGB(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int slice_no, unsigned int dest_pwidth, unsigned int dest_pheight,
		   float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, bool bg_sub, raw_data* raw=0);

    bool readToRGB(float* dest, unsigned int xpos, unsigned int ypos, unsigned int dest_width, unsigned int dest_height, unsigned int slice_no, 
		   float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, bool bg_sub, raw_data* raw=0);

    bool mip_projection(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int dest_pwidth, unsigned int dest_pheight,
		       float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw=0);

    bool mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
			float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw=0);
    bool readToFloat(float* dest, int xb, int yb, int zb, int pw, int ph, int pd, unsigned int waveIndex);  // reads a block of voxels into destination.. 
    bool readToFloatPro(float* dest, int xb, int iwidth, int yb, int iheight, int wave);     // the projection (but checks to make sure no negativ values).. 
    // wave in readtofloatPro is waveindex (and it is checked, but nothing happens if too large) 
    // we then also need a whole load of accessor functions to allow us to make sense of the data
    float xpos(){
	return(x);
    }
    float ypos(){
	return(y);
    }
    float width(){
	return(w);
    }
    float height(){
	return(h);
    }
    float depth(){
	return(d);
    }
    int pwidth(){
	return(pw);
    }
    int pheight(){
	return(ph);
    }
    void borders(int& left, int& right, int& bottom, int& top);
    int sectionNo(){
	return(frameNo);
    }
    int channelNo(){
	return(wave_no);
    }
    int channel(unsigned int pos){
	if(pos < wave_no){
	    return(waves[pos]);
	}
	return(0);
    }
    std::vector<float> channels(){
	std::vector<float> ch(wave_no);
	for(uint i=0; i < wave_no; ++i){
	    ch[i] = float(waves[i]);    // this is ugly, but hey, it's not my fault..
	}
	return(ch);
    }
    fluorInfo channelInfo(unsigned int pos);
    std::vector<overlap_data*> overlaps(){
	return(overlapData);
    }

    private :

	void determineZOffsets();    // this is actually a bit tricky .. 
    
    std::map<float, std::map<float, FrameStack*> > frames;      // since this is inconvenient to access I'll have an accessor function
    std::set<fluorInfo> flInfo;        // the various file sets and things.. 
    // the first photoSensor value for a given channel. 
    // set the values for each frame.. 
    std::map<fluorInfo, float> photoSensors;  
    std::vector<float> x_positions;
    std::vector<float> y_positions;
    std::set<float> x_set;
    std::set<float> y_set;              
    std::vector<float> z_positions;
    std::set<float> z_set;
    // use the sets for the initial settting up of the rectangle..
    // then use finalise to set up the vectors (these may not be necessary at the moment, but..)

    int* wave_lengths;
    unsigned int wave_no;      // the wavelengths that we'll be using.. 
    std::vector<int> waves;       // just so I can sort it.. 
    std::vector<overlap_data*> overlapData;  // somewhere to hide overlap data.. 

    float frameWidth, frameHeight;
    int pixelHeight, pixelWidth;  // this refers to the frame dimensions and should be the same for all frames.. 
    float maxIntensity;           //
    const char* fileName;               // set this at the first time we add a frame.. -- but default to 0 so we know if its been set

    float x, y;   // the beginning of the area covered (the lowest values by default)..
    float w, h;   // the width and height of the total area..
    int pw, ph;   // the pixel width of the whole image (as given by the w/pixel_size.. -- set these in finalise.. 
    float d;      // the depth, -but set this in initialise.. along with w, h and complete rectangle.. 
    int frameNo;  // the number of frames, set in finalise.. 
    bool completeRectangle;  // do we have complete coverage of a the area .. -this is set in finalise.. 

    int ifstreamCounter;

}; 
    

#endif
