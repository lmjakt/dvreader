#include "mt_rgb_lookup.h"
#include <iostream>

using namespace std;

mt_rgb_lookup::mt_rgb_lookup(unsigned int lu_size)
  : QThread(), lu_size(lu_size)
{
  resetVars();
}

mt_rgb_lookup::~mt_rgb_lookup()
{
}

void mt_rgb_lookup::toRGB(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
			  float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
			  unsigned int read_width, unsigned int read_height, 
			  float* lu_red, float* lu_green, float* lu_blue, float* c_map)
{
  src = source;
  contribMap = c_map;
  source_x = s_x;
  source_y = s_y;
  source_w = s_w;
  dst = dest;
  dst_x = d_x;
  dst_y = d_y;
  dst_w = d_w;
  r_width = read_width;
  r_height = read_height;

  r_lut = lu_red;
  g_lut = lu_green;
  b_lut = lu_blue;

  start();
}

void mt_rgb_lookup::run()
{
  if(!dst || !src || !r_width)
    return;
  if(dst_w < r_width || source_w < r_width)
    return;
  if(!contribMap)
    return( toRGB_direct() );
  if(contribMap)
    return( toRGB_contrib() );
}

void mt_rgb_lookup::resetVars()
{
  src = 0;
  contribMap = 0;
  source_w = 0;
  dst = 0;
  dst_w = 0;
  r_width = 0;
  r_height = 0;  
}

void mt_rgb_lookup::toRGB_direct()
{
  
  for(unsigned int dy=0; dy < r_height; ++dy){
    unsigned short* s = src + source_x + (source_y + dy) * source_w;
    float* d = dst + dst_x + (dst_y + dy) * dst_w;
    for(unsigned int dx=0; dx < r_width; ++dx){
      d[0] = r_lut[ *s ];  
      d[1] = g_lut[ *s ];
      d[2] = b_lut[ *s ];
      d += 3;
      ++s;
    }
  }
}

void mt_rgb_lookup::toRGB_contrib()
{
  cout << "toRGB_contrib source : " << source_x << "," << source_y << " : " << source_w
       << "\tdest : " << dst_x << "," << dst_y << " : " << dst_w
       << "\tread : " << r_width << "," << r_height << endl;
  for(unsigned int dy=0; dy < r_height; ++dy){
    unsigned short* s = src + source_x + (source_y + dy) * source_w;
    float* cm = contribMap + source_x + (source_y + dy) * source_w; // this isn't actually true at the moment
    float* d = dst + 3 * (dst_x + (dst_y + dy) * dst_w);
    for(unsigned int dx=0; dx < r_width; ++dx){
      d[0] += *cm * r_lut[ *s ];
      d[1] += *cm * g_lut[ *s ];
      d[2] += *cm * b_lut[ *s ];
      d += 3;
      ++s;
      ++cm;
    }
  }
}
