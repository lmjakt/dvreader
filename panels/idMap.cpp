#include "idMap.h"
#include <string.h>
#include <iostream>

using namespace std;

IdMap::IdMap(int x, int y, uint w, uint h){
  width = (int)w;
  height = (int)h;
  x_pos = x;
  y_pos = y;
  map = 0;
  if(w && h){
    map = new ulong[width * height];
    memset((void*)map, 0, sizeof(ulong) * width * height);
  }
}

IdMap::~IdMap(){
  delete map;
}

bool IdMap::setId(ulong id, int x, int y, int w, int h){
  if(w <= 0 || h <= 0)
    return(false);
  if( x >= x_pos + width || y >= y_pos + height)
    return(false);
  if(x + w < x_pos || y + h < y_pos)
    return(false);
  x = x < x_pos ? x_pos : x;
  y = y < y_pos ? y_pos : y;
  w = x + w > (x_pos + width) ? (x_pos + width) - x : w;
  h = y + h > (y_pos + height) ? (y_pos + height) - y : h;

  for(int yp=y; yp < (y + h); ++yp){
    for(int xp=x; xp < (x + w); ++xp){
      map[ yp * width + xp ] |= id;
    }
  }
  return(true);
}

// returns whether id is defined at the given positionn
bool IdMap::count(ulong id, int x, int y){
  // The check are written stupidly, because of some bug in the emacs formatting
  if(x < x_pos || x >= x_pos + width)
    return(false);
  if(y_pos >= y || y >= y_pos + height)
    return(false);
     
  return( map[ y * width + x] & id );
}

ulong IdMap::id(int x, int y){
  return( map[ y * width + x ] );
}

// returns the number of assigned ids at x, y
int IdMap::count(int x, int y){
  if(x < x_pos || x >= x_pos + width)
    return(0);
  if(y_pos >= y || y >= y_pos + height)
    return(0);
  int count = 0;
  ulong check = 1;
  ulong target = map[ y * width + x ];
  while(check){
    if(check & target)
      ++count;
    check <<= 1;
  }
  return(count);
}

void IdMap::resetMap(){
  memset((void*)map, 0, sizeof(long) * width * height);
}

void IdMap::reset(int x, int y, uint w, uint h){
  delete map;
  x_pos = x;
  y_pos = y;
  width = (int)w;
  height = (int)h;
  map = new ulong[w * h];
  memset((void*)map, 0, sizeof(long) * width * height);
}

void IdMap::pos(int& x, int& y){
  x = x_pos;
  y = y_pos;
}

void IdMap::dims(int& w, int& h){
  w = width;
  h = height;
}

float* IdMap::paintCountsToRGB(int& w, int& h, float maxCount){
  w = width;
  h = height;
  float* image = new float[3 * w * h];
  float* dst = image;
  float v;
  for(int y=0; y < h; ++y){
    for(int x=0; x < w; ++x){
      v = float(count(x, y)) / maxCount;
      *dst++ = v;
      *dst++ = v;
      *dst++ = v;
    }
  }
  return(image);
}
