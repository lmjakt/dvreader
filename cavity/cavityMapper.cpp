#include "cavityMapper.h"
#include "ballMap.h"

using namespace std;

CavityMapper::CavityMapper(ImageData* id, unsigned int wi,
			   int xr, int yr, int zr,
			   float maxP, float maxDirP)
{
  image = id;
  image->dims(width, height, depth);
  ballMap = new VolumeMap<CavityBall*>(width, height, depth);
  waveIndex=wi;
  xRadius=xr; yRadius=yr; zRadius=zr;
  maxPenalty=maxP;
  maxDirPenalty=maxP;

  // We may not use a background at all, but let's pretend.
  background = new Background(image, 16, 16, 8, 50);

  makeBalls();
}

CavityMapper::~CavityMapper(){
  //  map<o_set, CavityBall*> balls = ballMap->allBalls();
  //for(map<o_set, CavityBall*>::iterator it=balls.begin();
  for(vector<CavityBall*>::iterator it=balls.begin();
      it != balls.end(); ++it){
    delete (*it);
  }
  delete image;
  delete background;
  delete ballMap;
}

vector< vector<o_set> > CavityMapper::ballMembers(){
  vector< vector<o_set> > ballMembers;
  //map<o_set, CavityBall*> allBalls = ballMap->allBalls();
  //  for(map<o_set, CavityBall*>::iterator it=allBalls.begin();
  for(vector<CavityBall*>::iterator it=balls.begin();
      it != balls.end(); ++it){
    ballMembers.push_back((*it)->points());
  }
  return(ballMembers);
}

void CavityMapper::makeBalls(){
  float checkValue = 0;
  // make a Temproary cavity Ball for the surfaces
  CavityBall* tempBall = new CavityBall(image, background, ballMap,
					waveIndex, xRadius, yRadius, zRadius,
					maxPenalty, maxDirPenalty);
  tempBall->makeBall();
  std::vector<pos> surface;
  std::vector<pos> volume;
  tempBall->ballPoints(surface, volume);
  for(uint z=(uint)zRadius; 
      z < ((uint)depth - zRadius);
      ++z){
    for(uint y=(uint)yRadius;
	y < ((uint)height - yRadius);
	++y){
      for(uint x=(uint)xRadius;
	  x < ((uint)width - xRadius);
	  ++x){
	if(ballMap->masked(x, y, z))
	   continue;
	image->point(checkValue, x, y, z, waveIndex);
	if( checkValue > maxPenalty)
	  continue;
	CavityBall* cb = new CavityBall(image, background, ballMap,
					waveIndex, xRadius, yRadius, zRadius,
					maxPenalty, maxDirPenalty);
	cb->findCavity(x, y, z, surface, volume);
	//cout << ballMap.size() << endl;
	//	cout << x << "," << y << "," << z << "\t" << ballMap.size() << "\t" << &ballMap << "\t";
	// cb->printBallStats();
	if(!cb->points().size()){
	  delete cb;
	}else{
	  balls.push_back(cb);
	}
      }
    }
  }
  delete tempBall;
}

