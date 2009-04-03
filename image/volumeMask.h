#ifndef VOLUMEMASK_H
#define VOLUMEMASK_H

#include "bitMask.h"

class VolumeMask {
 public:
    VolumeMask(unsigned long width, unsigned long height, unsigned long depth);
    ~VolumeMask();
    bool setMask(bool b, unsigned long x, unsigned long y, unsigned long z);
    bool mask(unsigned long x, unsigned long y, unsigned long z){
	return( bitMask->bit( (z * w * h) + (y * w) + x ) );
    }
    void printMask();  // mostly for debugging reasons

 private:
    BitMask* bitMask;
    unsigned long maskSize;
    unsigned long w, h, d;
};

#endif
