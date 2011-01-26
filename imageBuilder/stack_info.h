#ifndef STACK_INFO_H
#define STACK_INFO_H

#include <vector>

struct stack_info {
  int x, y, z;
  unsigned int w, h, d;
  std::vector<unsigned int> channels;
  stack_info(){
    x = y = z = 0;
    w = h = d = 0;
  }
  stack_info(unsigned int wi, int xp, int yp, int zp,
	     unsigned int width, unsigned int height, unsigned int depth){
    x = xp; y = yp; z = zp;
    w = width; h = height; d = depth;
    channels.push_back(wi);
  }
  bool friend operator ==(const stack_info& a, const stack_info& b){
    return( a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w && a.h == b.h && a.d == b.d );
  }
  bool friend operator !=(const stack_info& a, const stack_info& b){
    return( !(a == b) );
  }
};

#endif
