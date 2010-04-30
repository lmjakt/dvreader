#ifndef TWO_D_BACKGROUND_H
#define TWO_D_BACKGROUND_H

#include "../datastructs/a_pos.h"


class Two_D_Background
{
 public:
  Two_D_Background();
  ~Two_D_Background();

  bool setBackground(float qnt, int cw, int ch, int w, int h, float* data);
  bool setBackground(float qnt, int cw, int ch, int w, int h, unsigned short* data);
  
  // inline this to make it faster (eg if making a whole image background slice)
  float bg(int x, int y){
    int xb = (x - cell_width/2) / cell_width;
    int yb = (y - cell_height/2) / cell_height;
    
    // as long as x and y are not negative, then the smallest value we'll get here will
    // be -0.5, which will be rounded to 0. So this should be a safe way of finding the appropriate
    // points from which to interpolate.
    
    // xb and yb can be 0, but we need these to be smaller than the the width and height of the background.
    // note that this is not error checking as these are allowed values, but for which we need to make
    // some compensation.
    xb = xb < bg_w - 1 ? xb : bg_w - 2;
    yb = yb < bg_h - 1 ? yb : bg_h - 2;
    int pb = yb * bg_w + xb;
    
    float bot = background[pb] + ((float)(x - bg_pos[pb].x) / (float)cell_width) * (background[pb+1] - background[pb]);
    float top = background[pb + bg_w] + ((float)(x - bg_pos[pb+bg_w].x) / (float)cell_width) * (background[pb+bg_w+1] - background[pb+bg_w]);
    float b = bot + ((float)(y - bg_pos[pb].y)/(float)cell_height) * (top - bot);
    return(b);
  }
  
 private:
  int width, height;
  int cell_width, cell_height;
  float qntile;
  float* background;
  int bg_w, bg_h;
  a_pos* bg_pos;
};

#endif
