#include "gaussian.h"
#include <math.h>
#include <string.h>
#include <iostream>

using namespace std;

// The gaussian distribution according to http://en.wikipedia.org/wiki/Gaussian_function
// 
// f(x) = a * e^(-(x^2/2c^2))
// where c is the variance (var)
// The below function takes a constant for 2/c^2, and refers to this simply
// as the radius of the gaussian. 
// Note that the blurring is carried out in squares, rather than in circles, since this
// is computationally easier.

float* gaussian_blur_2d(float* image, unsigned int w, unsigned int h, unsigned int radius)
{
  float* g_sq = gaussian_square(radius);
  float* b_image = new float[w * h];
  memset((void*)b_image, 0, sizeof(float) * w * h);
  // The easiest options, which I'll do for now, suggest leaving
  // the edges empty. We can maybe fix this later, but for now let's
  // be stupid.
  unsigned int d = 1 + 2 * radius;
  float* dest;
  float* gsq_source;
  float* source; 
  for(uint y=0; y < h - d; ++y){
    dest = b_image + (y + radius) * w + radius;
    for(uint x=0; x < w - d; ++x){
      gsq_source = g_sq;
      for(uint dy=0; dy < d; ++dy){
	source = image + (dy + y) * w + x;
	for(uint dx=0; dx < d; ++dx){
	  *dest += *source * *gsq_source;
	  ++source;
	  ++gsq_source;
	}
      }
      ++dest;
    }
  }
  delete []g_sq;
  return(b_image);
}

float* gaussian_blur_1d(float* image, unsigned int w, unsigned int h, unsigned int radius)
{
  if(!w || !h || !radius || radius >= w || radius >= h)
    return(0);
  float* line = gaussian_line(radius);
  float* b1_image = new float[w * h];
  float* b2_image = new float[w * h];
  memset((void*)b1_image, 0, sizeof(float) * w * h);
  memset((void*)b2_image, 0, sizeof(float) * w * h);
  unsigned int d = 1 + 2 * radius;
  float* source;
  float* dest;
  // I'm doing it wrong here. What I should do is blur in the x-direction,
  // then blur the partially blurred image in the y direction.. (not as below)

  // first in the x direction
  for(uint y=0; y < h - d; ++y){
    dest = b1_image + ((y+radius) * w) + radius;
    for(uint x=0; x < w - d; ++x){
      source = image + ((y + radius) * w) + x;
      for(uint dx=0; dx < d; ++dx){
	(*dest) += (*source) * line[dx];
	++source;
      }
      ++dest;
    }
  }
  // then in the y direction..
  for(uint y=0; y < h - d; ++y){
    dest = b2_image + ((y + radius) * w) + radius;
    for(uint x=0; x < w - d; ++x){
      source = b1_image + (y * w) + x + radius;
      for(uint dy=0; dy < d; ++dy){
	(*dest) += (*source) * line[dy];
	source += w;
      }
      ++dest;
    }
  }
  delete []line;
  delete []b1_image;
  return(b2_image);
}

float* gaussian_square(unsigned int radius)
{
  if(!radius){
    float* sq = new float[1];
    *sq = 1.0;
    return(sq);
  }
  int r = (int)radius;
  unsigned int d = 1 + radius * 2;
  unsigned int l = (1 + 2 * radius) * (1 + 2 * radius);
  float* sq = new float[l];
  float sqsum = 0;
  float var = (float)r;
  for(int dy = -r; dy <= +r; ++dy){
    for(int dx = -r; dx <= +r; ++dx){
      float delta = sqrt( (dy * dy) + (dx * dx) );
      sq[ (r + dy) * d + (r + dx) ] = expf(-(delta * delta)/var);
      sqsum += sq[ (r + dy) * d + (r + dx) ];
    }
  }
  // Make sure that the square sums to 1
  for(uint i=0; i < l; ++i)
    sq[i] /= sqsum;
  return(sq);
}

float* gaussian_line(unsigned int radius)
{
  if(!radius){
    float* line = new float[1];
    *line = 1.0;
    return(line);
  }
  int r = (int)radius;
  unsigned int d = 1 + radius * 2;
  float* line = new float[d];
  float var = (float)r;
  float sum = 0;
  for(int d = -r; d <= +r; ++d){
    line[ d + r ] = expf(-(d * d)/var);
    sum += line[ d + r ];
  }
  // make sure weights add up to 1.
  for(uint i=0; i < d; ++i)
    line[i] /= sum;
  return(line);
}
