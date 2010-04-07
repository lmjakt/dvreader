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

#include "frame.h"
#include "frameSet.h"
#include "fileSetInfo.h"
#include "../dataStructs.h"
#include <map>
#include <vector>
#include <fstream>

class Background;

class FrameStack {
    public :
	enum POSITION {
	    LEFT, TOP, RIGHT, BOTTOM
	};
    FrameStack(int* waveLengths, int waveNo, std::ifstream* inStream, float maxLevel);
    ~FrameStack();
    
    bool addFrame(const char* fname, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
			 short numInt, short numFloat, unsigned short byteSize,
			 bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz, Frame*& frame);   

    bool addFrame(Frame* frame);
    void setBackgrounds(std::map<fluorInfo, Background*> backgrounds);
    // returns 0, if the frame is added to this stack
    // otherwise returns another frameStack
    std::ifstream* fileStream(){
	return(in);
    }

    void finalise(float maxLevel, FrameInfo* frameData=0);  // sets upt the vector.. 
    void printFrames();

    // some accessor functions..
    float x_pos(){
	return(x);
    }
    float y_pos(){
	return(y);
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


    // and a function for setting the neighbour
    bool setNeighbour(FrameStack* neibour, int pos, bool recip=true);   // pos is 0, 1, 2, 3 going from left neighbour to bottom neighbour in clockwise order
    std::vector<overlap_data*> adjustNeighbourPositions(unsigned int secNo, unsigned int rolloff, int instep, int window, int px, int py);         // do something smart to check the relative positions of the neighbours.. .. px and py are the panel positions of the panel being called.. 
//    std::vector<overlap_data*> adjustNeighbourPositions(unsigned int secNo, float wavelength, unsigned int rolloff, int instep, int window, int px, int py);         // do something smart to check the relative positions of the neighbours.. .. px and py are the panel positions of the panel being called.. 
    offsets findNeighbourOffset(FrameStack* neighbour, float* neighbourArea, int nx1, int ny1, float* thisArea, int x1, int y1, int& areaW, int& areaH);  // goes through all colors.. 

    void determineFocalPlanes(unsigned int rolloff);  // determine the focal planes for each frameStack.. 
    void adjustPosition(int dx, int dy, POSITION n, float setAdjustmentFlag=0);
    
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
//    void adjustPosition(float dx, float dy);


    // only works if the neighbours have been set.. 
    // rolloff is the rolloff area created by the deconvolution..
    // instep is the number of pixels within this rolloff position that we will take the line from
    // window is the +/- area that we'll search 
    // This function does no errorchecking of the position, but assuming that the relative position is correct it does something.. 
    bool readToRGB(float* dest, float xpos, float ypos, float dest_width, 
		   float dest_height, unsigned int dest_pwidth, 
		   unsigned int dest_pheight, unsigned int slice_no,
		   std::vector<channel_info> chinfo, raw_data* raw=0);
    //		   float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, bool bg_sub, raw_data* raw=0);

    bool readToRGB(float* dest, int xpos, int ypos, 
		   unsigned int dest_width, unsigned int dest_height, unsigned int slice_no, 
		   std::vector<channel_info> chinfo, raw_data* raw=0); 
    //		   std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, bool bg_sub, raw_data* raw=0);


    bool mip_projection(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int dest_pwidth, unsigned int dest_pheight,
			float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw=0);

    bool mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
			float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw=0);

    // the below function is used when determining the position of the frame stack
    // do not use it for other purposes.
    bool readToFloat(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, 
    		     unsigned int iheight, unsigned int secNo, unsigned int waveIndex, float maxLevel);   // simply read the appropriate pixels in.. 

    bool readToFloat(float* dest, int xb, int iwidth, int yb, 
		     int iheight, int zb, int idepth,  unsigned int waveIndex, float maxLevel);   // simply read the appropriate pixels into a volume.. 
    bool readToShort(unsigned short* dest,unsigned int xb, unsigned int iwidth, unsigned int yb, 
		     unsigned int iheight, unsigned int secNo, unsigned int waveIndex);
      
    bool readToFloatPro(float* dest, unsigned int xb, unsigned int iwidth, unsigned int yb, 
			unsigned int iheight, unsigned int wave);   // simply read the appropriate pixels in.. 

    bool readToFloatProGlobal(float* dest, int xb, int iwidth, int yb, 
			      int iheight, unsigned int wave);   // convert global coordinates to local ones.. 


    float** projectionData(){
//	return(projection);
	return(frameInformation->projection);
    }

    // destination in mip_projection is an RGB float formatted array which has been transformed.
    // raw_data if defined should have enough space for all the appropriate wavelengths and pixels

    private :
    std::map<float, FrameSet*> sectionMap;   // this will organise everything into some sort of reasonable state..
    std::vector<FrameSet*> sections;         // do I actually use this.. ? 
    std::ifstream* in;
    
    // but actually .. we have ..
    FrameInfo* frameInformation;              // which we can make or get from file.. 
    float** projection;                      // some projection of all of the data.. (set in the finalise option..)

    FrameStack* leftNeighbour;
    FrameStack* topNeighbour;
    FrameStack* bottomNeighbour;
    FrameStack* rightNeighbour;    // these are set 
    float leftOverlap, topOverlap, rightOverlap, bottomOverlap;   // these refer to the corresponding neighbours.. 
    int leftBorder, rightBorder, topBorder, bottomBorder;         // these refer to global coordinates.. 
    
    float x, y;
    float width, height;   // taken from the first constructor..
    float z_begin, z_end;  // sections ordered..
    unsigned int pWidth, pHeight;   // pixel height and pixel number.
    int pixelX, pixelY;    // the begin position in pixel coordinates (this has to be set by the owner as frameStack itself can't know
    float maxIntensity;

    int* wave_lengths;
    int wave_no;            // for making frameSets...
    std::vector<float> fwaves;  // for useful things.. 

    bool finalised;
    float positionAdjusted;    // 
    bool neighboursAdjusted;

    std::vector<std::vector<float> > contrasts;   // one vector for each wavelength.. -- determines the focal planes.. 
    std::vector<float> focalPlanes;     // assigned by some equation.. 

    // a function that determines whether this stack or the neighbour stack gets to move..
    void adjustPosition(FrameStack* neibor, int dx, int dy, float corr);   // the rules for who moves whom are a bit complicated, hence.. 
                                  // this functionality overlaps with the 
    float* make_mip_projection(unsigned int wi, float maxLevel, std::vector<float>& contrast);  // just give the wavelength..  

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
