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

#ifndef COLORMAPPER_H
#define COLORMAPPER_H

#include <qthread.h>

class ColorMapper : public QThread
//class ColorMapper
{

    public :
	ColorMapper();
    void mapSingle(float* source, int W, int H, int XO, int YO, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, bool add=true);   // assume offsets calcluated
    void mapMerged(float* source, int W, int H, int* XO, int* YO, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, int* mColors, int mSize, bool add=true); // use a single merged thingy
    void mapMergedIndividualComponents(float* source, int W, int H, int* XO, int* YO, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, int* mColors, int mSize, bool add=true); // adjust mapping somehow..

    private :
	void init(float* source, int pixNo, float* dest, float* frameDest, int frameInterval, float* color, float* bias, float* scale, bool add);   // as this is always the same.. 
    void run();   // the public functions above basically set some variables and then call start.. or run, which calls one of the below functions... 
    void map_single();
    void map_merged();
    void map_merged_individual_components();

    // the variables needed...
    int mappingState;       // should make this an enum, but for now, 1 -> mapSingle, 2 -> mapMerged, 3 -> mapMergedIndividualComponents
    bool additive;          // do we add a colour or do we subtract..
    float* dataSource;
    int w, h, xo, yo;   // width height xoffset, yoffset (width and height are of the whole thing.. 
    int* xof;
    int* yof;           // needed for the merged colours.. 
    int pixelNo;
    float* destData;
    float* frameDestData;
    int frameIntervalNo;   // the number of individual thingies calculated..
    float* colors;          // treated differentely by the different functions, with no error checking..
    float* biasFactors;
    float* scaleFactors;     // again in single mode we just assume we have a pointer to the appropriate one, but when merge mapping with individual components we need to be a bit smarter..
    // and exclusively used for merging functions ..
    int* mergedOffsets;
    int* mergedColors;      // 
    int mergedSize;
};

#endif
