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

//#include "frameStack.h"
#include "../dataStructs.h"
#include "../datastructs/channelOffset.h"
#include "stack_stats.h"
#include "../imageBuilder/stack_info.h"
//#include "fileSetInfo.h"
//#include "../image/background.h"
#include <map>
#include <vector>
#include <string>
#include <QPoint>

/////////// the following introduces circular dependancies 
class Background;
class ImageData;
class BorderInfo;
class IdMap;
class FrameStack;
class FileSetInfo;
class ImStack;
class SLookup;

// Background objects contain a reference to an imageData object; the imageData object
// provides some higher level access functions for the FileSet Object, and has a pointer
// to a FileSet object. Hence we get..

// FileSet(1) --> Background --> ImageData --> FileSet(1)
// note that ImageData doesn't have to point to the same fileSet, though, since
// we so far only support one FileSet in a given application, this seems likely to
// be the case.

// So who deletes whom.?? 
// well, the background object doesn't delete the imagedata object, and the ImageData object
// doesn't delete the fileSet object
// so we might be ok.
// but this seems to be a big kludge to avoid making big changes in the flow of information.
//
// if we're not careful we could obviously end up introducing infinite loops. That would be bad.

class FileSet {
    public :
  FileSet(int* waveLengths, int waveNo, float maxLevel, int xy_margin);
    ~FileSet();

    bool addFrame(std::string fname, std::ifstream* in, std::ios::pos_type framePos, 
		  std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
		  short numInt, short numFloat, unsigned short byteSize,
		  bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz);
    bool getStack(float& xpos, float& ypos);   // sets the appropriate values up for a given thingy.. 
    bool finalise();   // checks for a complete rectangle and sets up the x, y, and z_position vectors
    void adjustStackPosition(float xp, float yp, QPoint P);
    void setPosMap();
    void setPanelBias(unsigned int waveIndex, unsigned int column, unsigned int row, float scale, short bias); 
    void setBackgroundPars(unsigned int waveIndex, int xm, int ym, float qnt, bool bg_subtract);
    bool setChannelOffsets(std::vector<ChannelOffset> offsets);
    void adjustStackBorders();
    bool updateFileSetInfo();

    bool readToRGB(float* dest, unsigned int xpos, unsigned int ypos, 
		   unsigned int dest_width, unsigned int dest_height, 
		   unsigned int slice_no, std::vector<channel_info> chinfo,
		   raw_data* raw=0);

    bool mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
			float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw=0);

    ImStack* imageStack(std::vector<unsigned int> wave_indices, bool use_cmap);
    ImStack* imageStack(stack_info sinfo, bool use_cmap);
    ImStack* imageStack(std::vector<unsigned int> wave_indices, int x, int y, int z,
			unsigned int w, unsigned int h, unsigned int d, bool use_cmap);
    bool readToFloat(float* dest, int xb, int yb, int zb, int pw, int ph, int pd, unsigned int waveIndex, bool use_cmap=false);  // reads a block of voxels into destination.. 
    bool readToShort(unsigned short* dest, int xb, int yb, unsigned int slice, int pw, int ph, unsigned int waveIndex);
    // the following reads the whole frame into dest; c and r are row and colum respectively.. 
    bool readToShort(unsigned short* dest, unsigned int c, unsigned int r, unsigned int slice, unsigned int waveIndex);
    
    // get some stats for a specified frame stack. c and r are column and row as above.. 
    stack_stats stackStats(unsigned int c, unsigned int r, int xb, int yb, 
			   int s_width, int s_height, unsigned int waveIndex);
    stack_stats stackStats(unsigned int c, unsigned int r, int xb, int yb, int zb,
			   int s_width, int s_height, int s_depth, unsigned int waveIndex);

    bool readToFloatPro(float* dest, int xb, int iwidth, int yb, int iheight, int wave);     // the projection (but checks to make sure no negativ values).. 
    // wave in readtofloatPro is waveindex (and it is checked, but nothing happens if too large) 
    // we then also need a whole load of accessor functions to allow us to make sense of the data
    void setBackgroundParameters(std::map<fluorInfo, backgroundPars> bgp);
    BorderInfo* borderInformation(float x, float y);
    float* paintCoverage(int& w, int& h, float maxCount);

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
    unsigned long plength(){
      return( (unsigned long)pw * (unsigned long)ph);
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

    std::set<fluorInfo> channelInfo(){
      return(flInfo);
    }
    FileSetInfo* panelInfo(){
      return(stackInfo);
    }

    void stackDimensions(int& col_no, int& row_no, int& panelWidth, int& panelHeight);
    private :

    void determineZOffsets();    // this is actually a bit tricky .. 
    void initBackgrounds();      // we should probably remove this at some point.
    void initBackgrounds(std::map<fluorInfo, backgroundPars> bgp);
    void setLookupTables(std::vector<channel_info>& ch_info);

    std::map<float, std::map<float, FrameStack*> > frames;      // since this is inconvenient to access I'll have an accessor function
    std::map<ulong, FrameStack*> frameIds;
    IdMap* framePosMap;
    std::set<fluorInfo> flInfo;        // the various file sets and things.. 
    // the first photoSensor value for a given channel. 
    // set the values for each frame.. 
    std::map<fluorInfo, SLookup*> luts;
    std::map<fluorInfo, float> photoSensors;  
    std::map<fluorInfo, Background*> backgrounds;
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
    FileSetInfo* stackInfo;   // holds the projection and so forth. Use for adjusting the projection data.

    float frameWidth, frameHeight;
    int pixelHeight, pixelWidth;  // this refers to the frame dimensions and should be the same for all frames.. 
    int rolloff;                  // the area lost due to the deconvolution. I'm not sure how to get this from the file at the moment
    float maxIntensity;           //
    int xyMargin;                 // needed to create framestacks.. 
    char* fileName;               // set this at the first time we add a frame.. -- but default to 0 so we know if its been set

    float x, y;   // the beginning of the area covered (the lowest values by default)..
    float w, h;   // the width and height of the total area..
    int pw, ph;   // the pixel width of the whole image (as given by the w/pixel_size.. -- set these in finalise.. 
    float d;      // the depth, -but set this in initialise.. along with w, h and complete rectangle.. 
    int frameNo;  // the number of frames, set in finalise.. 
    bool completeRectangle;  // do we have complete coverage of a the area .. -this is set in finalise.. 

    int ifstreamCounter;

}; 
    

#endif
