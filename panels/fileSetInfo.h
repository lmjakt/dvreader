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

#ifndef FILESETINFO_H
#define FILESETINFO_H

#include <map>
#include <vector>

// This holds information about the FrameStacks in a given FileSet. This includes things like 
// a projection of the sections (one for each frameStack) as well the corrected offsets for the
// fileStacks owned by the file set. (And ofcourse some ways of identifying the frameStacks..
const int magicNumber = 99;  // first number of the file.. (don't care about byte order)
const int maxStackNumber = 100;
const int maxPixelNo = 5000000;   // allow panels to contain up to 5 megapixels.. 

typedef unsigned int uint;

// these structs do not take ownership of any data. Hence they do not delete anything
// when deleted.

struct FrameInfo {
    unsigned int waveNo;
    float** projection;
    float xpos, ypos;     // the positions.. 
    int xp, yp; // the pixel positions.. 
    int width, height;
    
    FrameInfo(){
	waveNo = 0;
	projection = 0;
	xpos = ypos = 0;
	width = height = 0;
    }

    FrameInfo(unsigned int wn, float** p, float x_pos, float y_pos, int x, int y, int w, int h){
	waveNo = wn;
	projection = p;
	xpos = x_pos;
	ypos = y_pos;
	width = w;
	height = h;
	xp = x;
	yp = y;
    }
    ~FrameInfo(){}
};

struct FileSetInfo {
    FileSetInfo(){
	stackNo = 0;
    }
    FileSetInfo(std::vector<int> wl, unsigned int w, unsigned int h){
	waveLengths = wl;
	waveNo = wl.size();
	width = w;
	height = h;
	stackNo = 0;
    }
    ~FileSetInfo(){}
    FileSetInfo(const char* fname);  // read from a file ? 
    FrameInfo* getStack(float x, float y);  // returns a FrameInfo pointer or 0 on failure
    void addFrameInfo(FrameInfo* finfo, float x, float y); // add one .. 
    bool writeInfo(const char* fname);   // write stuff to this file.. 
    void dims(int& w, int& h);
    int image_width();
    int image_height();

    std::map<float, std::map<float, FrameInfo*> > stacks;
    std::vector<int> waveLengths;
    unsigned int width, height;  // useful to know.. 
    unsigned int waveNo;
    int stackNo;
    bool ok;

};

#endif
