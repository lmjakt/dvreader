#include "cavityMapper.h"
#include "ballMap.h"

using namespace std;

CavityMapper::CavityMapper(ImageData* id, unsigned int wi,
			   int xr, int yr, int zr,
			   float maxP, float maxDirP)
{
  image = id;
  image->dims(width, height, depth);
  ballMap = new BallMap(width, height, depth);
  waveIndex=wi;
  xRadius=xr; yRadius=yr; zRadius=zr;
  maxPenalty=maxP;
  maxDirPenalty=maxP;

  // We may not use a background at all, but let's pretend.
  background = new Background(image, 16, 16, 8, 50);

  makeBalls();
}

CavityMapper::~CavityMapper(){
  map<o_set, CavityBall*> balls = ballMap->allBalls();
  for(map<o_set, CavityBall*>::iterator it=balls.begin();
      it != balls.end(); ++it){
    delete (*it).second;
  }
  delete image;
  delete background;
  delete ballMap;
}

vector< vector<o_set> > CavityMapper::ballMembers(){
  vector< vector<o_set> > balls;
  map<o_set, CavityBall*> allBalls = ballMap->allBalls();
  for(map<o_set, CavityBall*>::iterator it=allBalls.begin();
      it != allBalls.end(); ++it){
    balls.push_back(it->second->points());
  }
  return(balls);
}

void CavityMapper::makeBalls(){
  for(uint z=(uint)zRadius; 
      z < ((uint)depth - zRadius);
      ++z){
    for(uint y=(uint)yRadius;
	y < ((uint)height - yRadius);
	++y){
      for(uint x=(uint)xRadius;
	  x < ((uint)width - xRadius);
	  ++x){
	CavityBall* cb = new CavityBall(image, background, ballMap,
					waveIndex, xRadius, yRadius, zRadius,
					x, y, z, maxPenalty, maxDirPenalty);
	//cout << ballMap.size() << endl;
	//	cout << x << "," << y << "," << z << "\t" << ballMap.size() << "\t" << &ballMap << "\t";
	// cb->printBallStats();
	if(!cb->points().size())
	  delete cb;
      }
    }
  }
}

