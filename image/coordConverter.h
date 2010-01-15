#ifndef COORDCONVERTER_H
#define COORDCONVERTER_H

// a small functor that converts from linear to volume coordinates

#include <iostream>
#include "../dataStructs.h"

class CoordConverter
{
 private:
    o_set w, h, d;
    o_set s;
 public:
    CoordConverter(int width, int height, int depth){
	w = (o_set)width;
	h = (o_set)height;
	d = (o_set)depth;
	s = w * h * d;
    }
    CoordConverter(unsigned long width, unsigned long height, unsigned long depth){
	w = width; height = h; d = depth;
	s =w * h * d;
    }
    unsigned long linear(unsigned long x, unsigned long y, unsigned long z){
	return( z * w * h + y * w + x);
    }
    unsigned long linear(int x, int y, int z){
	return( z * w * h + y * w + x );
    }
    void vol(unsigned long p, int& x, int& y, int& z){
	z = int( p / (w * h) );
	y = int( (p % (w * h)) / h);
	x = int( p % w );
    }

};


#endif
