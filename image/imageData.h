#ifndef IMAGEDATA_H
#define IMAGEDATA_H

// Simplified and perhaps optimised access to random voxels within the data.
// For use by image analysing objects and functions.

// This is essentially the good bits taken out of imageAnalyser.h
// In order to provide a better organisation.

#include "volumeCache.h"
#include "../panels/fileSet.h"

const int pCache = 1024000;  // if we were really smart we could get that from /proc/cpuinfo

typedef unsigned int uint;

class ImageData {
 public:
    ImageData(FileSet* fs, uint interpolate=1);
    ~ImageData();

    void dims(unsigned int& w, unsigned int& h, unsigned int& d);  // the dimensions of the thing.
    void dims(int& w, int& h, int& d);  // the dimensions of the thing.    
    bool point(float& p, int xp, int yp, int zp, unsigned int wi);   // tries the volume_cache, if not ok, makes a new cache.. 

    uint interp_no(){
	return(interp);
    }

 private:
    FileSet* data;
    float* interpBuffer;
    uint interp;
    VolumeCache* volume;
    int cache_size;
    int cache_width;
    int cache_height;
    int cache_depth;

    // the owner can request a change of cache size
    int rc_width, rc_height, rc_depth;

    // interpolated values
    int width, height, depth; 

    // functions
    void readData(int x, int y, int z, uint wi);
    void makeCache();
};
    
#endif
