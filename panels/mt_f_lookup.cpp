#include "mt_f_lookup.h"

mt_f_lookup::mt_f_lookup(unsigned int lu_size)
  : QThread(), lu_size(lu_size)
{
  src = 0;
  source_w = 0;
  dst = 0;
  dst_w = 0;
  r_width = 0;
  r_height = 0;
}

mt_f_lookup::~mt_f_lookup()
{
}

void mt_f_lookup::toFloat(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
		     float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
		     unsigned int read_width, unsigned int read_height, float* lu_table)
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
  lut = lu_table;
  
  run();
}

void mt_f_lookup::run()
{
  if(!dst || !src || !r_width)
    return;
  if(dst_w < r_width || source_w < r_width)
    return;

  for(unsigned int dy=0; dy < r_height; ++dy){
    unsigned short* s = src + source_x + (source_y + dy) * source_w;
    float* d = dst + dst_x + (dst_y + dy) * dst_w;
    for(unsigned int dx=0; dx < r_width; ++dx){
      *d = lut[ *s ];  // this could probably be written as *(d++) = lut[ *(s++) ] ut I have to check
      ++d;
      ++s;
    }
  }
}
