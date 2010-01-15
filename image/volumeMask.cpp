#include "volumeMask.h"
#include <iostream>

VolumeMask::VolumeMask(unsigned long width, unsigned long height, unsigned long depth){
    w = width;
    h = height;
    d = depth;
    maskSize = w * h * d;
    
    bitMask = new BitMask(maskSize);
}

VolumeMask::~VolumeMask(){
    delete bitMask;
}

bool VolumeMask::setMask(bool b, unsigned long x, unsigned long y, unsigned long z){
    if(x >= w || y >= h || z >= d)
	return(false);
    return( bitMask->set_bit(b, (z * w * h) + (y * w) + x ) );
}

bool VolumeMask::setMask(bool b, int x, int y, int z){
    return( setMask(b, (unsigned long)x, (unsigned long)y, (unsigned long)z) );
}

bool VolumeMask::setMask(bool b, unsigned long o){
  return( bitMask->set_bit(b, o) );
}

void VolumeMask::printMask(){
    for(unsigned long z = 0; z < d; ++z){
	std::cout << "Slice " << z << std::endl;
	for(unsigned long y = 0; y < h; ++y){
	    for(unsigned long x=0; x < w; ++x){
		std::cout << mask(x, y, z);
	    }
	    std::cout << std::endl;
	}
	std::cout << std::endl;
    }
}

void VolumeMask::zeroMask(){
    bitMask->zeroMask();
}
