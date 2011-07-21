#include "mt_rgb_lookup.h"

mt_rgb_lookup::mt_rgb_lookup(unsigned int lu_size)
  : QThread(), lu_size(lu_size)
{
  src = 0;
  source_w = 0;
  dst = 0;
  dst_w = 0;
  r_width = 0;
  r_height = 0;
}

mt_rgb_lookup::~mt_rgb_lookup()
{
}

void mt_rgb_lookup::toRGB(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
		     float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
		     unsigned int read_width, unsigned int read_height, 
		     float* lu_red, float* lu_green, float* lu_blue)
{
  src = source;
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

  run();
}

void mt_rgb_lookup::run()
{
  if(!dst || !src || !r_width)
    return;
  if(dst_w < r_width || source_w < r_width)
    return;
  
  for(unsigned int dy=0; dy < r_height; ++dy){
    unsigned short* s = src + source_x + (source_y + dy) * source_w;
    float* d = dst + dst_x + (dst_y + dy) * dst_w;
    for(unsigned int dx=0; dx < r_width; ++dx){
      d[0] = r_lut[ *s ];  // this could probably be written as *(d++) = lut[ *(s++) ] ut I have to check
      d[1] = g_lut[ *s ];
      d[2] = b_lut[ *s ];
      d += 3;
      ++s;
    }
  }
}
