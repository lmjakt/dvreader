#ifndef GAUSSIAN_H
#define GAUSSIAN_H

typedef unsigned int uint;

float* gaussian_blur_2d(float* image, unsigned int w, unsigned int h, unsigned int radius);
float* gaussian_blur_1d(float* image, unsigned int w, unsigned int h, unsigned int radius);

// makes a square with width and height of 1 + 2*radius
float* gaussian_square(unsigned int radius);
float* gaussian_line(unsigned int radius);

#endif
