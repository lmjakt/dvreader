#ifndef MT_RGB_LOOKUP_H
#define MT_RGB_LOOKUP_H

#include <QThread>

class mt_rgb_lookup : public QThread
{
 public:
  mt_rgb_lookup(unsigned int lu_size);
  ~mt_rgb_lookup();

  void toRGB(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
	     float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
	     unsigned int read_width, unsigned int read_height, 
	     float* lu_red, float* lu_green, float* lu_blue, float* c_map = 0);
  
 private:
  void run();
  void resetVars();    // call to make sure fresh variables each time.. 

  void toRGB_direct();
  void toRGB_contrib();  // uses a contribMap of the same dimensions as source

  // not very ideal structure of these things. A aimple C function using posix threads
  // seems like a better idea. I need to study posix threads..

  unsigned int lu_size;

  unsigned short* src;
  float* contribMap;
  unsigned int source_x;
  unsigned int source_y;
  unsigned int source_w;
  float* dst;
  unsigned int dst_x;
  unsigned int dst_y;
  unsigned int dst_w;
  unsigned int r_width;
  unsigned int r_height;

  float* r_lut;
  float* g_lut;
  float* b_lut;
  
};

#endif
