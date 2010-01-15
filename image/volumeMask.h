#ifndef VOLUMEMASK_H
#define VOLUMEMASK_H

#include "bitMask.h"
#include <iostream>
#include <stdlib.h>

class VolumeMask {
 public:
    VolumeMask(unsigned long width, unsigned long height, unsigned long depth);
    ~VolumeMask();
    bool setMask(bool b, unsigned long x, unsigned long y, unsigned long z);
    bool setMask(bool b, int x, int y, int z);
    bool setMask(bool b, unsigned long o);
    bool mask(unsigned long x, unsigned long y, unsigned long z){
	return( bitMask->bit( (z * w * h) + (y * w) + x ) );
    }
    bool mask(int x, int y, int z){
	return( mask( (unsigned long)x, (unsigned long)y, (unsigned long)z ) );
    }
    bool mask(unsigned long o){
      if(o >= (w * h * d)){
	std::cerr << "VolumeMask offset too large : " << o << std::endl;
	exit(1);
      }
      return( bitMask->bit(o) );
    }

    void printMask();  // mostly for debugging reasons
    void zeroMask();  // set all values to 0.

 private:
    BitMask* bitMask;
    unsigned long maskSize;
    unsigned long w, h, d;
};

#endif
