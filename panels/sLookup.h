#ifndef SLOOKUP_H
#define SLOOKUP_H

// class that converts a unsigned short* to either
// a float (toFloat) or to RGB, using a static lookup table.

// the actual conversion is done by member objects that run in
// separate threads but make use of the same lookup arrays.
// Since these will be used only as read only arrays should
// not be a major issue.

#include <vector>
#include "../dataStructs.h" // for channel_info

class mt_f_lookup;
class mt_rgb_lookup;

class SLookup 
{
  static const unsigned int max_ushort = 65536; // 2^16
 public:
  SLookup(float scale, float bias, float r, float g, float b, unsigned short thread_no, unsigned int mx=max_ushort);
  ~SLookup();

  void setPars(channel_info& chi);
  void setMx(unsigned int mx);
  void setPars(float scl, float bs);
  void setColor(float r, float g, float b);

  bool toFloat(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
	       float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
	       unsigned int read_width, unsigned int read_height);

  bool addToRGB_f(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
		  float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
		  unsigned int read_width, unsigned int read_height, float* contribMap);

  
 private:
  float scale;
  float bias;
  float red;
  float green;
  float blue;
  unsigned short thread_no;
  unsigned int mx_l;

  float lu_float[max_ushort];
  float lu_red[max_ushort];
  float lu_green[max_ushort];
  float lu_blue[max_ushort];

  std::vector<mt_f_lookup*> flooks;
  std::vector<mt_rgb_lookup*> rgblooks;

};

#endif
