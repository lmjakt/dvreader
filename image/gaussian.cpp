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


float* gaussian_blur_3d(float* image, unsigned int w, unsigned int h, unsigned int d, unsigned int radius)
{
  unsigned int diameter = 1 + radius * 2; 
  if(!w || !h || !d)
    return(0);

  float* line = gaussian_line(radius);
  float* cline = compensation_line(line, radius);
  float* b1_image = new float[w * h * d]; // reuse for the third blur.. 
  float* b2_image = new float[w * h * d];
  memset((void*)b1_image, 0, sizeof(float) * w * h * d);
  memset((void*)b2_image, 0, sizeof(float) * w * h * d);
  float *source, *dest;
  int r = (int)radius;
  // in the x direction. Do all layers and all y lines. 
  for(uint z=0; z < d; ++z){
    for(uint y=0; y < h; ++y){
      //      dest = b1_image + (z * w * h) + (y * w) + radius;
      dest = b1_image + (z * w * h) + (y * w);      
      for(int x=0; x < (int)w; ++x){
	int gb = x >= r ? 0 : r - x;
	int ge = (x + r) < (int)w ? diameter : (r + w - x);
	float comp = cline[ge - (gb+1)];
	source = image + (z * w * h) + (y * w) + x + (gb - r);  // horrible..    
	for(int g=gb; g < ge; ++g){
	  (*dest) += line[g] * (*source) ;
	  ++source;
	}
	(*dest) /= comp;
	++dest;
      }
    }
  }
  // in the y directions..
  for(uint z=0; z < d; ++z){
    for(int y=0; y < (int)h; ++y){
      int gb = y >= r ? 0 : r - y;
      int ge = (y + r) < (int)h ? diameter : (r + h - y);
      float comp = cline[ge - (gb+1)];
      dest = b2_image + (z * w * h) + (y * w);
      for(uint x=0; x < w; ++x){
	source = b1_image + (z * w * h) + ((y + (gb - r)) * w) + x;
	for(int g=gb; g < ge; ++g){
	  (*dest) += line[g] * (*source);
	  source += w;
	}
	(*dest) /= comp;
	++dest;
      }
    }
  }
  memset((void*)b1_image, 0, sizeof(float) * w * h * d);
  // and in the z-direction.
  for(int z=0; z < (int)d; ++z){
    int gb = z >= r ? 0 : r - z;
    int ge = (z + r) < (int)d ? diameter : (r + d - z);
    float comp = cline[ ge - (gb+1) ];
    cout << "z : " << z << " comp : " << ge << "-" << gb << "=" << ge-gb << " --> "  << comp << endl;
    for(uint y=0; y < h; ++y){
      dest = b1_image + (z * w * h) + (y * w);
      for(uint x=0; x < w; ++x){
	source = b2_image + ((z + (gb - r)) * w * h) + (y * w) + x;
	for(int g=gb; g < ge; ++g){
	  (*dest) += line[g] * (*source);
	  source += (w * h);
	}
	(*dest) /= comp;
	++dest;
      }
    }
  }
  delete []b2_image;
  delete []line;
  delete []cline;
  return(b1_image);
}


float* gaussian_deblur_3d(float* image, unsigned int w, unsigned int h, unsigned int d, unsigned int radius)
{
  unsigned int diameter = 1 + radius * 2; 
  if(!w || !h || !d)
    return(0);

  float* line = gaussian_line(radius);
  float* cline = compensation_line(line, radius);
  float* b1_image = new float[w * h * d]; // reuse for the third blur.. 
  float* b2_image = new float[w * h * d];
  memcpy((void*)b1_image, (void*)image, sizeof(float) * w * h * d);
  //  memset((void*)b1_image, 0, sizeof(float) * w * h * d);
  //  memset((void*)b2_image, 0, sizeof(float) * w * h * d);
  float *source, *dest;
  int r = (int)radius;
  // in the x direction. Do all layers and all y lines. 
  for(uint z=0; z < d; ++z){
    for(uint y=0; y < h; ++y){
      //      dest = b1_image + (z * w * h) + (y * w) + radius;
      dest = b1_image + (z * w * h) + (y * w);      
      for(int x=0; x < (int)w; ++x){
	int gb = x >= r ? 0 : r - x;
	int ge = (x + r) < (int)w ? diameter : (r + w - x);
	source = image + (z * w * h) + (y * w) + x + (gb - r);  // horrible..
	(*dest) *= 2.0;
	for(int g=gb; g < ge; ++g){
	  (*dest) -= line[g] * (*source);
	  ++source;
	}
	(*source) = (*source) >= 0 ? (*source) : 0;
	++dest;
      }
    }
  }
  // in the y directions..
  memcpy((void*)b2_image, (void*)b1_image, sizeof(float) * w * h * d);
  for(uint z=0; z < d; ++z){
    for(int y=0; y < (int)h; ++y){
      int gb = y >= r ? 0 : r - y;
      int ge = (y + r) < (int)h ? diameter : (r + h - y);
      dest = b2_image + (z * w * h) + (y * w);
      for(uint x=0; x < w; ++x){
	source = b1_image + (z * w * h) + ((y + (gb - r)) * w) + x;
	(*dest) *= 2.0;
	for(int g=gb; g < ge; ++g){
	  (*dest) -= line[g] * (*source);
	  source += w;
	}
	(*dest) = (*dest) > 0 ? (*dest) : 0;
	++dest;
      }
    }
  }
  memcpy((void*)b1_image, (void*)b2_image, sizeof(float) * w * h * d);
  //  memset((void*)b1_image, 0, sizeof(float) * w * h * d);
  // and in the z-direction.
  for(int z=0; z < (int)d; ++z){
    int gb = z >= r ? 0 : r - z;
    int ge = (z + r) < (int)d ? diameter : (r + d - z);
    for(uint y=0; y < h; ++y){
      dest = b1_image + (z * w * h) + (y * w);
      for(uint x=0; x < w; ++x){
	source = b2_image + ((z + (gb - r)) * w * h) + (y * w) + x;
	(*dest) *= 2.0;
	for(int g=gb; g < ge; ++g){
	  (*dest) -= line[g] * (*source);
	  source += (w * h);
	}
	(*dest) = (*dest) > 0 ? (*dest) : 0;
	++dest;
      }
    }
  }
  delete []b2_image;
  delete []line;
  delete []cline;
  return(b1_image);
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

// the sums ..
float* compensation_line(float* line, unsigned int radius)
{
  unsigned int d = 1 + 2 * radius;
  float* c_line = new float[ d ]; // use cline[ ge - gb ]
  memset((void*)c_line, 0, sizeof(float) * 2 * radius);
  for(unsigned int l=0; l < d; ++l){
    for(unsigned int i=0; i < l; ++i)
      c_line[l] += line[i];
  }
  return(c_line);
}
