#include "PointDilater.h"
#include "imStack.h"
#include <stdlib.h>
#include <iostream>

// At the moment we implement this as a functor object,
// as there is no particular need for state to be remembered.
// And in fact good reason not to remember any state

PointDilater::PointDilater()
{
}

PointDilater::~PointDilater()
{
}

ImStack* PointDilater::dilate(int xp, int yp, int zp,
			      int w, int h, int d,
			      std::vector<blob_set> blobs,
			      int xr, int yr, int zr)
{
  // we use int rather than unsigned int for w, h, d and so on
  // because we will need to add and sum and compare these and
  // I'm still not completely sure as to the different effects
  // of mixing signed and unsigne variables.
  // apparently signed numbers --> unsigned numbers
  // when mixed. That's not what we'd want. Oh well.

  w = abs(w);
  h = abs(h);
  d = abs(d);
  
  xr = abs(xr);
  yr = abs(yr);
  zr = abs(yr);
  
  if(!w || !h || !d)
    return(0);

  std::vector<int> x_off;
  std::vector<int> y_off;
  std::vector<int> z_off;

  int max_r = xr > yr ? xr : yr;
  max_r = max_r > zr ? max_r : zr;
  
  // the below would appear to fail when a specified radius
  // is 0, as if that is the case only a 0 distance in that
  // dimension should be allowed and distances ought to be multiplied
  // by inf rather than 0. However, in that case the following loop will
  // only supply 0 values, so that shoudln't be a problem.
  float x_mult = xr ? float(max_r)/float(xr) : 0;
  float y_mult = yr ? float(max_r)/float(yr) : 0;
  float z_mult = zr ? float(max_r)/float(zr) : 0;
    
  x_mult = x_mult * x_mult;
  y_mult = y_mult * y_mult;
  z_mult = z_mult * z_mult;

  std::cout << "setting up offsets: " << std::endl;
  for(int x = -xr; x <= xr; ++x){
    for(int y = -yr; y <= yr; ++y){
      for(int z = -zr; z <= zr; ++z){
	float d = x*x*x_mult + y*y*y_mult + z*z*z_mult;
	if(d <= (max_r*max_r)){
	  x_off.push_back(x);
	  y_off.push_back(y);
	  z_off.push_back(z);
	}
      }
    }
  }

  // and now we are are ready to make the image stack.
  float* im_data = new float[w * h * d];
  memset((void*)im_data, 0, sizeof(float) * w * h * d);
  
  int x, y, z;
  std::cout << "going through blobs: " << blobs.size() << std::endl;
  for(unsigned int i=0; i < blobs.size(); ++i){
    if(!(i % 1000))
      std::cout << i << " / " << blobs.size() << std::endl;
    blobs[i].mg_pos(x, y, z);
    //    if((x < xp + w) && (x >= xp) && (y < yp + h) && (y >= yp) && (z < zp + d && z >= zp)){
    if(xp > (x+xr) || (x-xr) > (xp+w)) 
      continue;
    if(yp > (y+yr) || (y-yr) > (yp+h))
      continue;
    if(zp > (z+zr) || (z-zr) > (zp+d))
      continue;
    for(unsigned int j=0; j < x_off.size(); ++j){
      int xn = x + x_off[j];
      int yn = y + y_off[j];
      int zn = z + z_off[j];
      if((xn < xp + w) && (xn >= xp) && (yn < yp + h) && (yn >= yp) && (zn < zp + d && zn > zp))
	im_data[ (zn-zp) * w * h + (yn-yp) * w + (xn-xp) ] += 0.05;
    }
  } 
  // at this point it might be useful to know the maximum value of the data so that we can
  // interpret the colours within it.
  float max_level = im_data[0];
  for(int i=1; i < (w * h * d); ++i)
    max_level = max_level < im_data[i] ? im_data[i] : max_level;

  channel_info ch_info;
  ch_info.maxLevel = max_level;
  std::cout << "And the maxLevel is: " << max_level << std::endl;
  
  ImStack* imStack = new ImStack(im_data, ch_info, xp, yp, zp, w, h, d);
  return(imStack);
}
