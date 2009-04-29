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

#ifndef IMAGEANALYSER_H
#define IMAGEANALYSER_H

#include "volumeCache.h"
#include "../panels/fileSet.h"
#include "../dataStructs.h"
#include "../spotFinder/perimeter.h"
#include <vector>
#include <iostream>

// this class contains a number of functions for analysing images derived from a given fileSet
// These have been placed here in order to allow the object to have a number of different caches
// that can be used when accessing data from the fileSet

// It might be an idea to try and keep individual caches within the size of the processor cache
// --and then try to make sure that we design things a bit carefully.. 
const int processorCache = 1024000;  // if we were really smart we could get that from /proc/cpuinfo
const int min_point_height = 100;
const int min_point_depth = 14;    // these should be big enough to include everything around a point.. 
                             // however, it would be good if they were tunable a little bit more easily
                             // actually make them big enough to try and include more than one point.. 

struct PerimeterData {
    parameterData p_data;
    std::vector<PerimeterSet> perimeterData;
    PerimeterData(){
    }
    PerimeterData(parameterData pd, std::vector<PerimeterSet> perd){
	p_data = pd;
	perimeterData = perd;
    }
};


class ImageAnalyser 
{
    
    public :
	ImageAnalyser(FileSet* fs);
    ~ImageAnalyser();
    
    void dims(int& w, int& h, int& d);  // the dimensions of the thing.
    void dims(unsigned long& w, unsigned long& h, unsigned long& d);  // the dimensions of the thing.

    bool simpleLine(float* line, int xb, int yb, int zb, int l, Dimension dim, unsigned int wi);   // tries the appropriate cache first, if not then makes a new cache and returns the line
    bool point(float& p, int xp, int yp, int zp, unsigned int wi);                                   // tries the volume_cache, if not ok, makes a new cache.. 
    threeDPeaks* findAllPeaks(unsigned int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm);   // the same as above but goes through all the slices.. 
    threeDPeaks* findAllPeaks_3D(unsigned int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm);   // uses a 3 dimensional peak recognition based on peeling onions
    std::vector<Nucleus> findNuclei(float* source, unsigned int w, unsigned int h, float minValue);        // take the float as an argument since deltaviewer keeps a copy of the projection
    parameterData findContrasts(float* source, unsigned int w, unsigned int h);
    parameterData findSets(float* source, unsigned int w, unsigned int h, int minSize, int maxSize, float minValue);
    PerimeterData findPerimeters(float* source, unsigned int w, unsigned int h, int minSize, int maxSize, float minValue);

    // some convenience functions..
    std::vector<std::vector<float> > x_line(int y_pos, int z_pos);
    std::vector<std::vector<float> > y_line(int x_pos, int z_pos);

    std::vector<std::vector<float> > mip_xline(int ypos);
    std::vector<std::vector<float> > mip_yline(int xpos);   // these two functions don't use the cache since they deal with projection data which is in memory

    std::vector<float*> mip_areas(int xb, int yb, int width, int height);
    std::vector<float*> mip_areas(int& width, int& height);     // returns the projections for the whole image and sets width and height to the appropriate values.

    void blur(float* values, float* dest, uint w, uint h, int r, double sigma, double order);

    private :
	FileSet* data;             // this contains the appropriate set of pointers that allow us to get hold of data. 
    VolumeCache* area_cache;    // for getting y-lines
    VolumeCache* x_z_slice_cache;   // for getting z-lines
    VolumeCache* volume_cache;      // for refining drops.. 
    ssize_t cache_size;             // make all caches this size.. (make it about 1/2 the size of the processor cache)
    ssize_t area_cache_width;
    ssize_t x_z_slice_cache_width;
    ssize_t x_z_slice_cache_height;    // work these out in the constructor.. 

    ssize_t vol_cache_width;


    std::vector<linearPeak> findPeaks(float* line, int length, int pr, float minPeakValue, float maxEdgeValue, int dim);  // also dumb. doesn't know how to define positions.. 
    void adjustPeaks(std::vector<linearPeak>& peaks, int bx, int by, int bz, Dimension dim);
    std::vector<Nucleus> findNuclearPerimeters(std::vector<line>& lines);
    Nucleus findNuclearPerimeter(const std::set<line>& lines);


    // set defintion of nuclei.. ? from some paper or other.

    // defines the set in the mask by extending to all neighbours higher than the starting position.. 
    // this is a recursive function to allow the expansion.. 
    void expandSet(float* source, char* mask, unsigned int w, unsigned int h, unsigned int x, unsigned int y, int& setSize, float& setSum, int maxSize, std::vector<int>& members, float minValue);  
    std::vector<int> expandPerimeter(float* source, char* mask, unsigned int w, unsigned int h, float minValue, int origin, int maxSize, int& minX, int& minY, int& perWidth, int& perHeight);
    void fillPerimeter(float* dest, unsigned int w, unsigned int h, std::vector<int>& perimeter, int minX, int minY, int perWidth, int perHeight);
    void floodFill(char* mask, int perWidth, int perHeight, char bvalue, char fvalue, int x, int y);

    float absf(float v){     // make this an inline function for speed rather than calling the fabsf library function.. (but that might be a bad idea as just setting the sign bit is likely to be faster ?
	if(v > 0){
	    return(v);
	}
	return(-v);
    }

};


#endif
