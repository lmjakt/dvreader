#ifndef VOLUMEMAP_H
#define VOLUMEMAP_H

#include "volumeMask.h"
#include "linMap.h"
#include "blob.h"
#include <map>
#include <vector>

// one day I'll learn how to use templates..

//typedef blob ptr;  // this is defined in the linMap.h
typedef unsigned int uint;
typedef unsigned long ulong;

class VolumeMap {
 public:
    VolumeMap();
    VolumeMap(int width, int height, int depth);
    ~VolumeMap();

    bool insert(off_set pos, ptr* obj);
    bool insert(int x, int y, int z, ptr* obj);

    // these give 0 if not masked or out of bounds.
    ptr* value(off_set pos);
    ptr* value(int x, int y, int z);

    bool masked(int x, int y, int z){
	return(mask->mask(x, y, z));
    }

    off_set memSize();
    off_set mapSize();

    void clear();

 private:
    // use one map for each x-line in the sample.
    // and a bitwise mask for quick checks.

//    std::vector< std::map<int, ptr*> > lines;
    std::vector< LinMap*> lines;
    VolumeMask* mask;

    unsigned long size;
    unsigned int lineNo;
    // the dimensions deliberately use signed integers.
    int w, h, d;  

    off_set linear(int x, int y, int z){
	return( (off_set)z * (off_set)w * (off_set)h + (off_set)y * (off_set)w + (off_set)x );
    }
    void toVol(off_set p, int& x, int& y, int& z){
	z = p / (w * h);
	y = (p % (w * h)) / w;
	x = p % w;
    }

    bool outOfBounds(int x, int y, int z){
	return(x >= w || y >= h || z >= d || x < 0 || y < 0 || z < 0);
    }
};

#endif
