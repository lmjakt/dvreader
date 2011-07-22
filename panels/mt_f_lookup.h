#ifndef MT_F_LOOKUP_H
#define MT_F_LOOKUP_H

#include <QThread>

class mt_f_lookup : public QThread
{
 public :
  mt_f_lookup(unsigned int lu_size);
  ~mt_f_lookup();

  void toFloat(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
	       float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
	       unsigned int read_width, unsigned int read_height, float* lu_table);
 private:
  void run();
  
  // since run cannot be called with any parameters, we need to set them here.
  // this is bit problematic, as run can be called by the public function start()
  // might want to consider overriding qthread start() to do nothing.
  
  unsigned int lu_size;

  unsigned short* src;
  unsigned int source_x;
  unsigned int source_y;
  unsigned int source_w;
  float* dst;
  unsigned int dst_x;
  unsigned int dst_y;
  unsigned int dst_w;
  unsigned int r_width;
  unsigned int r_height;
  float* lut;
};

#endif
