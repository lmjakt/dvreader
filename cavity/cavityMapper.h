#ifndef CAVITYMAPPER_H
#define CAVITYMAPPER_H

#include "../dataStructs.h"
#include "cavityBall.h"
//#include "ballMap.h"
#include "../image/background.h"
#include "../image/imageData.h"
#include "../image/volumeMap.h"
#include <vector>
#include <map>

class CavityMapper
{
 public:
  CavityMapper(ImageData* id, unsigned int wi,
	       int xr, int yr, int zr,
	       float maxP, float maxDirP);
  ~CavityMapper();

  std::vector< std::vector<o_set> > ballMembers();

 private:
  VolumeMap<CavityBall*>* ballMap;
  std::vector<CavityBall*> balls;
  //  BallMap* ballMap;
  //  std::map<o_set, CavityBall*> ballMap;
  ImageData* image;
  Background* background;
  int xRadius, yRadius, zRadius;
  unsigned int width, height, depth;
  unsigned int waveIndex;
  float maxPenalty, maxDirPenalty;

  void makeBalls();
};

#endif
