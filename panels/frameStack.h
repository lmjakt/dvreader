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

#ifndef FRAMESTACK_H
#define FRAMESTACK_H

//#include "frame.h"
//#include "frameSet.h"
#include "fileSetInfo.h"
#include "../dataStructs.h"
#include "../datastructs/channelOffset.h"
#include "borderInformation.h"
#include "stack_stats.h"
#include <map>
#include <vector>
#include <fstream>

class Background;
class IdMap;
class Frame;
class FrameSet;
class SLookup;
struct panel_bias;

class FrameStack {
    public :
  //	enum POSITION {
  //	    LEFT, TOP, RIGHT, BOTTOM
  //	};
  FrameStack(int* waveLengths, int waveNo, std::ifstream* inStream, float maxLevel, int xy_margin);
    ~FrameStack();
    
    bool addFrame(const char* fname, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
			 short numInt, short numFloat, unsigned short byteSize,
			 bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz, Frame*& frame);   

    bool addFrame(Frame* frame);
    void setBackgrounds(std::map<fluorInfo, Background*> backgrounds);
    void setPanelBias(unsigned int wi, float scale, short bias);
    void setBackgroundPars(unsigned int wi, int xm, int ym, float qnt, bool bg_subtract);
    bool setChannelOffsets(std::vector<ChannelOffset> offsets);
    void setLookupTables(std::map<fluorInfo, SLookup*>* luts);
    // returns 0, if the frame is added to this stack
    // otherwise returns another frameStack
    std::ifstream* fileStream(){
	return(in);
    }

    void finalise(float maxLevel, FrameInfo* frameData=0);  // sets upt the vector.. 
    void printFrames();

    bool contains_pixel(int x, int y);
    bool bleachCount_g(int x, int y, unsigned int& count);

    // some accessor functions..
    float x_pos(){
	return(real_x);
    }
    float y_pos(){
	return(real_y);
    }
    float imageWidth(){
	return(width);
    }
    float imageHeight(){
	return(height);
    }
    int frameNo(){
	return(sectionMap.size());
    }
    float lolap(){
	return(leftOverlap);
    }
    float tolap(){
	return(topOverlap);
    }
    float rolap(){
	return(rightOverlap);
    }
    float bolap(){
	return(bottomOverlap);
    }
    unsigned int p_width(){
	return(pWidth);
    }
    unsigned int p_height(){
	return(pHeight);
    }
    void setPixelPos(int xp, int yp, bool setBorder=false){
	pixelX = xp;
	pixelY = yp;
	if(setBorder){
	    leftBorder = xp;
	    rightBorder = xp + pWidth;
	    bottomBorder = yp;
	    topBorder = yp + pHeight;
	}
	if(frameInformation){             // whoa this is ugly .. 
	    frameInformation->xp = pixelX;
	    frameInformation->yp = pixelY;
	}
    }
    FrameInfo* frameInfo(){
	return(frameInformation);
    }
    void setContribMap(float* map);
    void setContribMap(IdMap* idmap, ulong id);  // DEPRECATED GET RID OF IF THE ABOVE THING WORKS
    int left(){
	return(pixelX);
    }
    int right(){
	return(pixelX + pWidth);
    }
    int bottom(){
	return(pixelY);
    }
    int top(){
	return(pixelY + pHeight);
    }
    int left_border(){
	return(leftBorder);
    }
    int right_border(){
	return(rightBorder);
    }
    int top_border(){
	return(topBorder);
    }
    int bottom_border(){
	return(bottomBorder);
    }

    float isPositionAdjusted(){
	return(positionAdjusted);
    }
    bool adjustedNeighbours(){
	return(neighboursAdjusted);
    }
    int setBorder(int pos, POSITION n); // POSITION refers to the position of the boundary that I'm setting for myself.. 
    // returns the position that should be set by the caller.. (i.e. we negotiate a position)... 
    // AT THE MOMENT THIS FUNCTION is trusting and doesn't check to make sure the numbers make any sense.. but.. 

    void setBorders();  // uses neighbours to set the borders. Should be called after all adjustments
    int nearestBorderGlobal(int x, int y);

    // and a function for setting the neighbour
    bool setNeighbour(FrameStack* neibour, int pos, bool recip=true);   // pos is 0, 1, 2, 3 going from left neighbour to bottom neighbour in clockwise order
    // do something smart to check the relative positions of the neighbours.. .. px and py are the panel positions of the panel being called.. 
    std::vector<overlap_data*> adjustNeighbourPositions(unsigned int secNo, unsigned int rolloff, int instep, int window, int px, int py);         

    offsets findNeighbourOffset(FrameStack* neighbour, float* neighbourArea, int nx1, int ny1, float* thisArea, int x1, int y1, int& areaW, int& areaH); 

    void determineFocalPlanes(unsigned int rolloff);  // determine the focal planes for each frameStack.. 
    void adjustPosition(int dx, int dy, POSITION n, float setAdjustmentFlag=0);
    void adjustPosition(QPoint p);   // the rules for who moves whom are a bit complicated, hence.. 
    
    std::vector<std::vector<float> > contrastData(){    // this allows the parent to work stuff out.. 
	return(contrasts);
    }
    std::vector<float> float_waves(){
	return(fwaves);
    }
    bool setFocalPlane(unsigned int wIndex, float plane){
	if(wIndex < focalPlanes.size()){
	    focalPlanes[wIndex] = plane;
	    return(true);
	}
	return(false);
    }

    // respects channelOffsets
    bool readToRGB(float* dest, int xpos, int ypos, 
		   unsigned int dest_width, unsigned int dest_height, unsigned int slice_no, 
		   std::vector<channel_info> chinfo, raw_data* raw=0); 

    // respects channelOffsets
    bool mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
			float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw=0);

    // the below function is used when determining the position of the frame stack
    // do not use it for other purposes. (it does not call global_to_local, requires local coordinates?)
    bool readToFloat(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, 
    		     unsigned int iheight, unsigned int secNo, unsigned int waveIndex, float maxLevel);   // simply read the appropriate pixels in.. 

    // respects channelOffsets
    bool readToFloat(float* dest, int xb, int iwidth, int yb, 
		     int iheight, int zb, int idepth,  unsigned int waveIndex, float maxLevel, bool use_cmap=false);   // simply read the appropriate pixels into a volume.. 
    // respects channelOffsets
    bool readToShort(unsigned short* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, 
		     unsigned int iheight, unsigned int secNo, unsigned int waveIndex);

    // reads the whole frame info. Assumes that dest is the right size.
    bool readToShort(unsigned short* dest, unsigned int secNo, unsigned int waveIndex);
      
    // return stack_stats for a subset of the image (to reduce the size of data needed to calculate stuff.
    stack_stats stackStats(int xb, int yb, int s_width, int s_height, unsigned int waveIndex);
    stack_stats stackStats(int xb, int yb, int zb, int s_width, int s_height, int s_depth, unsigned int waveIndex);

    bool readProjectionData(float* dest, uint d_width, uint d_x, uint d_y,
			    uint xb, uint yb, uint c_width, uint c_height, uint wave);

    bool readToFloatProGlobal(float* dest, int xb, int iwidth, int yb, 
			      int iheight, unsigned int wave);   // convert global coordinates to local ones.. 


    float** projectionData(){
//	return(projection);
	return(frameInformation->projection);
    }

    BorderInfo* borderInformation();
    BorderArea* borderArea(POSITION pos);
    BorderArea* borderArea(FrameStack* nbor, int x, int y, int h, int w);
    // direct access to neighbors
    const FrameStack* left_neighbour();
    const FrameStack* right_neighbour();
    const FrameStack* top_neighbour();
    const FrameStack* bottom_neighbour();
    

    // Bleaching compensation related functions
    void clearBleachCount();
    void incrementBleachCount(int x, int y, int radius, unsigned int count);
    unsigned int* bleachCounts_g(int x, int y, int w, int h);  // returns the bleach information. x and y in global coordinates. 
    float bleachCountsMap_f(float* map, int map_x, int map_y, int map_w, int map_h); // returns the max count 

    private :
    std::map<unsigned int, panel_bias*> panelBiasMap; // set when setting things.
    std::map<float, FrameSet*> sectionMap;   // this will organise everything into some sort of reasonable state..
    std::vector<FrameSet*> sections;         // do I actually use this.. ? 
    std::ifstream* in;
    
    unsigned int* bleach_count;            // the number of times a pixel has been exposed prior to this imaging.

    // but actually .. we have ..
    FrameInfo* frameInformation;              // which we can make or get from file.. 
    float* contribMap;                        // defines what proportion of positions should be taken from this framestack.
    float** projection;                      // some projection of all of the data.. (set in the finalise option..)

    FrameStack* leftNeighbour;
    FrameStack* topNeighbour;
    FrameStack* bottomNeighbour;
    FrameStack* rightNeighbour;    // these are set 
    float leftOverlap, topOverlap, rightOverlap, bottomOverlap;   // these refer to the corresponding neighbours.. 
    int leftBorder, rightBorder, topBorder, bottomBorder;         // these refer to global coordinates.. 
    
    float real_x, real_y;  // the physical location
    float width, height;   // taken from the first constructor..
    float z_begin, z_end;  // sections ordered..
    unsigned int pWidth, pHeight;   // pixel height and pixel number.
    int pixelX, pixelY;    // the begin position in pixel coordinates (this has to be set by the owner as frameStack itself can't know
    float maxIntensity;
    int margin;   // set to 0. use rolloff instead.
    int rolloff;  // set to the value specified for xymargin for the constructor.

    int* wave_lengths;
    int wave_no;            // for making frameSets...
    std::vector<float> fwaves;  // for useful things.. 
    std::vector<ChannelOffset> channelOffsets;
    bool finalised;
    float positionAdjusted;    // 
    bool neighboursAdjusted;

    std::vector<std::vector<float> > contrasts;   // one vector for each wavelength.. -- determines the focal planes.. 
    std::vector<float> focalPlanes;     // assigned by some equation.. 

    // a function that determines whether this stack or the neighbour stack gets to move..
                                  // this functionality overlaps with the 
    float* make_mip_projection(unsigned int wi, float maxLevel, std::vector<float>& contrast);  // just give the wavelength..  

    bool readToFloatPro(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, 
			unsigned int iheight, unsigned int wave);   // simply read the appropriate pixels in.. 

    // some functions for stuff..
    void normalise_y(float* values, unsigned int w, unsigned int h);  // normalises a chunk of data in the y lines.. 
    void normalise_x(float* values, unsigned int w, unsigned int h);  // normalises a chunk of data in the y lines.. 
    float correlate(float* a, float* b, unsigned int l);
    float meanValue(float* v, unsigned int l);
    float determineContrast(float* values, unsigned int w, unsigned int h, unsigned int xp, unsigned int yp, unsigned int sw, unsigned int sh);  // contrast being simply defined as the sum of (x(i) - x(i+1))^2 for all values..  (xp and so on indicate a sub area)
    bool globalToLocal(int xpox, int ypos, int dest_width, int dest_height,
		       int& dest_x, int& source_x, int& dest_y, int& source_y, unsigned int& subWidth, unsigned int& subHeight);   // change the global coordinates to the local coordinates that should be contributed to
    //                                                                 // return false if no overlap.. 
};
    


#endif
