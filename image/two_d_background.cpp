#include "two_d_background.h"
#include <vector>
#include <algorithm>

using namespace std;

Two_D_Background::Two_D_Background()
{
  width = height = cell_width = cell_height = 0;
  background = 0;
  bg_pos = 0;
  bg_w = bg_h = 0;
}

Two_D_Background::~Two_D_Background()
{
  delete []background;
  delete []bg_pos;
}

bool Two_D_Background::setParameters(float qnt, int cw, int ch)
{
  if(qnt < 0 || qnt > 1.0)
    return(false);
  if(ch < 0 || cw < 0)
    return(false);
  if(cell_width != cw || cell_height != ch || qnt != qntile){
    if(background){
      delete []background;
      background = 0;
    }
    if(bg_pos){
      delete []bg_pos;
      bg_pos = 0;
    }
  }

  cell_width = cw;
  cell_height = ch;
  qntile = qnt;
  return(true);
}

bool Two_D_Background::setBackground(int w, int h, unsigned short* data)
{
  return(setBackground(qntile, cell_width, cell_height, w, h, data));
}


bool Two_D_Background::setBackground(int w, int h, float* data)
{
  return(setBackground(qntile, cell_width, cell_height, w, h, data));
}

bool Two_D_Background::backgroundSet()
{
  return( background != 0 );
}

bool Two_D_Background::setBackground(float qnt, int cw, int ch, int w, int h, float* data)
{
  if(qnt < 0 || cw < 0 || ch < 0 || w < 0 || h < 0 ||
     cw > w || ch > w)
    return(false);
  width = w; height = h;
  cell_width = cw; cell_height = ch;
  delete []background;
  delete []bg_pos;
  
  bg_w = (width / cell_width);
  bg_h = (height / cell_height);
  int bg_size = bg_w * bg_h;
  int cell_size = cell_width * cell_height;
  // if qnt was set to 1 or more, than we have a problem.
  qntile = qnt < (float)(cell_size-1)/(float)(cell_size) ? qnt : (float)(cell_size-1)/(float)(cell_size);
  
  background = new float[bg_size];
  bg_pos = new a_pos[bg_size];

  for(int by=0; by < bg_h; ++by){
    int v_pos = by * cell_height;
    for(int bx=0; bx < bg_w; ++bx){
      int h_pos = bx * cell_width;
      vector<float> rect;
      rect.reserve(cell_width * cell_height);
      for(int dy=0; dy < cell_height && (dy + v_pos) < height; ++dy){
	for(int dx=0; dx < cell_width && (dx + h_pos) < width; ++dx)
	  rect.push_back( data[ (dy + v_pos)*width + (dx + h_pos)] );
      }
      sort(rect.begin(), rect.end());
      background[ by * bg_w + bx] = rect[ (unsigned int)( float(rect.size()) * qntile) ];
      bg_pos[by * bg_w + bx].x = (bx * cell_width) + cell_width/2;
      bg_pos[by * bg_w + bx].y = (by * cell_height) + cell_height/2;
    }
  }
  return(true);
}

bool Two_D_Background::setBackground(float qnt, int cw, int ch, int w, int h, unsigned short* data)
{
  if(w <= 0 || h <= 0)
    return(false);
  float* buffer = new float[ w * h ];
  float* e = buffer + (w * h);
  for(float* b=buffer; b < e; ++b){
    (*b) = (float)(*data);
    ++data;
  }
  bool ok = setBackground(qnt, cw, ch, w, h, buffer);
  delete(buffer);
  return(ok);
}
