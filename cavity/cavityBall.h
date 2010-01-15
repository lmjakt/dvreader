#ifndef CAVITYBALL_H
#define CAVITYBALL_H

#include "../image/background.h"
#include "../image/imageData.h"
#include "../image/volumeMask.h"
#include "../dataStructs.h"
#include "ballMap.h"
#include <map>
#include <vector>
#include <math.h>

// an expanded flood filling algorithm
// uses a ball shaped object to fill in areas
// in order not to cross borders with holes.
// There are probably smarter ways of doing this, but
// I'm not sure exactly how.


// This seems wrong. I can't think of a good way of doing this
// right now.
struct dirPenalty {
  float xn, xp;
  float yn, yp;
  float zn, zp;
  float total;
  
  dirPenalty(){
    xn = xp = yn = yp = zn = zp = 0;
    total = 0;
  }
  void addPenalty(float p, int dx, int dy, int dz){
    xn = (dx < 0) ? p + xn : xn;
    xp = (dx > 0) ? p + xp : xp;
    yn = (dy < 0) ? p + yn : yn;
    yp = (dy > 0) ? p + yp : yp;
    zn = (dz < 0) ? p + zn : zn;
    zp = (dz > 0) ? p + zp : zp;
    total += p;
  }
  float penalty(int dx, int dy, int dz){
    float p = 0;
    p = (dx < 0 && xn > p) ? xn : p;
    p = (dx > 0 && xp > p) ? xp : p;
    p = (dy < 0 && yn > p) ? yn : p;
    p = (dy > 0 && yp > p) ? yp : p;
    p = (dz < 0 && zn > p) ? zn : p;
    p = (dz > 0 && zp > p) ? zp : p;
    return(p);
  }
};


class CavityBall
{

 public:
  CavityBall(ImageData* id, Background* bg, BallMap* bmap,
	     unsigned int waveIndex,
	     int xr, int yr, int zr,
	     int x, int y, int z,
	     float maxP, float maxDirP);

  ~CavityBall();
  void printBallStats();
  std::vector<o_set> points(){
    return(members);
  }

 private:
  ImageData* image;
  Background* background;

  BallMap* ballMap;
  //  std::map<o_set, CavityBall*>* ballMap;
  std::vector<o_set> members;

  // voxels contained by the ball.
  std::vector<pos> surface;
  std::vector<pos> volume; 

  float maxDirPenalty;
  float maxPenalty;
  int xi, yi, zi;     // the initial positions
  int xc, yc, zc;     // the current position
  unsigned int wi;
  int width, height, depth; // the dimensions of the image.

  int xRadius, yRadius, zRadius;

  void init();
  void makeBall();
  void fillBall(std::set<pos>& tsurf);
  VolumeMask* makeMask(std::set<pos>& tsurf);
  VolumeMask* makeMask(std::vector<pos>& tsurf);
  void fillMask(VolumeMask* mask, int x, int y, int z);
  void expand(pos p, pos dp, dirPenalty dirP);
  bool isSurface(VolumeMask* mask, int x, int y, int z);

  o_set oset(pos& p){
    return( p.z * (width * height) + p.y * width + p.x);
  }
  void toVol(o_set o, int& x, int& y, int& z){
    z = o / (width * height);
    y = (o % (width * height) ) / width;
    x = o % width;
  }
};

#endif
