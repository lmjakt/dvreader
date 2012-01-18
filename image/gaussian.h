#ifndef GAUSSIAN_H
#define GAUSSIAN_H

typedef unsigned int uint;

////// DO NOT USE THESE FUNCTIONS IN INDEPENDENT THREADS!! 

// 2d and 1d do the same thing. 2d function should be removed.. as it's just a waste of time
// all of these return an image with margins of radius. (Hence radius needs to be not too big)
// This is especially true for the 3d function, where depth, may often be quite shallow.
float* gaussian_blur_2d(float* image, unsigned int w, unsigned int h, unsigned int radius);
// 2_d wraps 1_d, remove at some point.
float* gaussian_blur_1d(float* image, unsigned int w, unsigned int h, unsigned int radius);
float* gaussian_blur_3d(float* image, unsigned int w, unsigned int h, unsigned int d, unsigned int radius);

// subtract the surrounding region using a gaussian function. (negative values to 0).
float* gaussian_deblur_3d(float* image, unsigned int w, unsigned int h, unsigned int d, unsigned int radius);

// makes a square with width and height of 1 + 2*radius
float* gaussian_square(unsigned int radius);
float* gaussian_line(unsigned int radius);
float* compensation_line(float* line, unsigned int radius);

#endif
