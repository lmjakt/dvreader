#include "borderInformation.h"
#include "../dataStructs.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

BorderArea::BorderArea(){
  t_data = 0;
  n_data = 0;
  wave_lengths = 0;
  t_bleach_count = 0;
  n_bleach_count = 0;
}

BorderArea::BorderArea(float** td, float** nd, int* wave_l, int wave_n, int xp, int yp, int w, int h){
  t_data = td; n_data = nd; 
  x = xp; y = yp; width=w; height=h;
  wave_no=wave_n;
  wave_lengths = new int[wave_no];
  memcpy((void*)wave_lengths, (void*)wave_l, sizeof(int)*wave_no); 
  t_bleach_count = 0;
  n_bleach_count = 0;
}

BorderArea::BorderArea(float** td, float** nd, int* wave_l, int wave_n, uint* tbleach, uint* nbleach, int xp, int yp, int w, int h){
  t_data = td; 
  n_data = nd;
  t_bleach_count = tbleach;
  n_bleach_count = nbleach;
  x = xp; y = yp; width=w; height=h;
  wave_no=wave_n;
  wave_lengths = new int[wave_no];
  memcpy((void*)wave_lengths, (void*)wave_l, sizeof(int)*wave_no); 
}

BorderArea::~BorderArea(){
  delete t_data;
  delete n_data;
  delete wave_lengths;
  delete t_bleach_count;
  delete n_bleach_count;
}


BorderInfo::BorderInfo(float x, float y){
  offSet = QPoint(0, 0);
  bias = 0;
  scale = 1.0;
  x_pos = x;
  y_pos = y;
}

BorderInfo::~BorderInfo(){
    for(map<POSITION, BorderArea*>::iterator it=areas.begin();
	it != areas.end(); ++it)
      delete (*it).second;
    areas.clear();
}

void BorderInfo::setArea(BorderArea* ba, POSITION pos){
  if(!ba)
    return;
  if(areas.count(pos))
    delete areas[pos];
  areas[pos] = ba;
}

void BorderInfo::setBias(float b){
  bias = b;
}

void BorderInfo::setScale(float s){
  scale = s;
}

void BorderInfo::setScaleAndBias(float s, float b){
  scale = s;
  bias = b;
}
  


void BorderInfo::setOffset(QPoint p){
  offSet = p;
}

QPoint BorderInfo::offset(){
  return(offSet);
}

float* BorderInfo::paint_overlap(POSITION pos, int wl, int& w, int& h, color_map& t_color, color_map& n_color){
  if(!areas.count(pos)){
    w = h = 0;
    return(0);
  }
  BorderArea* ba = areas[pos];
  w = ba->width + abs(offSet.x());
  h = ba->height + abs(offSet.y());

  if(wl < 0 || wl >= ba->wave_no){
    cerr << "BorderInfo::paint_overlap wavelength index is negative or too large : " << wl << endl;
    w = h = 0;
    return(0);
  }

  float* image = new float[w * h * 3];
  memset((void*)image, 0, sizeof(float) * 3 * w * h);
  
  float* t_data = ba->t_data[wl];
  float* n_data = ba->n_data[wl];
  
  // the offset is specified for the current panel; but if the offset is negative,
  // then I need to apply a positive offset to the neighbour instead. (And that is
  // separately for the x and t dimension. Hence:

  QPoint t_off, n_off;
  t_off.rx() = offSet.x() < 0 ? 0 : offSet.x();
  t_off.ry() = offSet.y() < 0 ? 0 : offSet.y();
  
  n_off.rx() = offSet.x() > 0 ? 0 : -offSet.x();
  n_off.ry() = offSet.y() > 0 ? 0 : -offSet.y();
  
  for(int y = 0; y < ba->height; ++y){
    for(int x = 0; x < ba->width; ++x){
      int t_o = 3 * ((y + t_off.y()) * w + (x + t_off.x()));
      int n_o = 3 * ((y + n_off.y()) * w + (x + n_off.x()));
      float t_v = bias + scale * (*t_data);
      float n_v = bias + scale * (*n_data);
      image[t_o] += t_v * t_color.r;
      image[t_o + 1] += t_v * t_color.g;
      image[t_o + 2] += t_v * t_color.b;

      image[n_o] += n_v * n_color.r;
      image[n_o + 1] += n_v * n_color.g;
      image[n_o + 2] += n_v * n_color.b;
      ++t_data;
      ++n_data;
    }
  }
  return(image);
}

set<int> BorderInfo::channels(){
  set<int> ch;
  for(map<POSITION, BorderArea*>::iterator it=areas.begin();
      it != areas.end(); ++it){
    for(int i=0; i < (*it).second->wave_no; ++i)
      ch.insert((*it).second->wave_lengths[i]);
  }
  return(ch);
}
