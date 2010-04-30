#ifndef A_POS_H
#define A_POS_H
#include <math.h>

// background can do with a pos struct (area_pos = a_pos)
struct a_pos {
  int x, y;
  a_pos(){
    x = y = 0;
  }
  a_pos(int X, int Y){
    x = X; y = Y;
  }
  int dx(a_pos& b){
    return(x - b.x);
  }
  int dy(a_pos& b){
    return(y - b.y);
  }
  float dist(a_pos& b){
    return sqrt( float( (x - b.x)*(x - b.x) + (y - b.y)*(y - b.y) ) );
  }
};

#endif
