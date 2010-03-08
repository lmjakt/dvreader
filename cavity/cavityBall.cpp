#include "cavityBall.h"
#include "ballMap.h"
#include <iostream>
#include <algorithm>
#include <math.h>

using namespace std;

CavityBall::CavityBall(ImageData* id, Background* bg, VolumeMap<CavityBall*>* bmap,
		       unsigned int waveIndex,
		       int xr, int yr, int zr,
		       //		       int x, int y, int z,
		       float maxP, float maxDirP)
{
  //  cout << "cavity ball constructor" << endl;
  image = id;
  image->dims(width, height, depth);
  background = bg;
  ballMap = bmap;
  wi = waveIndex;
  xi = yi = zi = 0;
  //  xi = x; yi = y; zi = z;
  recurseCount = 0;
  xRadius = xr;
  yRadius = yr;
  zRadius = zr;

  maxPenalty=maxP;
  maxDirPenalty=maxDirP;

  //cout << "cavity ball constructor calling init()" << endl;
  // init();
  //cout << "cavity ball constructor done" << endl;
  // if(members.size()){
  //   cout << "\tballMap size : " << ballMap->mapSize() << "\t";
  //   printBallStats();
  // }
}


CavityBall::~CavityBall(){
  // Don't delete image or background as these ought to be shared by
  // the cavity balls (anything else would be terrible).
}

void CavityBall::findCavity(int x, int y, int z){
  xi = x;
  yi = y;
  zi = z;
  makeBall();
  init();
}

void CavityBall::findCavity(int x, int y, int z,
			    std::vector<pos>& surf, std::vector<pos>& vol){
  xi = x;
  yi = y;
  zi = z;
  surface = surf;
  volume = vol;
  init();
}

void CavityBall::printBallStats(){
  int xmax, xmin, ymax, ymin, zmax, zmin;
  int x, y, z;
  if(!members.size()){
    cerr << "CavityBall: empty ball" << endl;
    return;
  }
  o_set o = members[0];
  toVol(o, x, y, z);
  xmax = xmin = x;
  ymax = ymin = y;
  zmax = zmin = z;
  for(vector<o_set>::iterator it=members.begin(); it != members.end(); ++it){
    toVol((*it), x, y, z);
    xmax = x > xmax ? x : xmax;
    xmin = x < xmin ? x : xmin;
    ymax = y > ymax ? y : ymax;
    ymin = y < ymin ? y : ymin;
    zmax = z > zmax ? z : zmax;
    zmin = z < zmin ? z : zmin;
  }
  cout << "Cavity Ball volume : " << members.size()
       << "  occupies : (" 
       << xmin << "->" << xmax << "),("
       << ymin << "->" << ymax << "),("
       << zmin << "->" << zmax << ")" << endl;
}

void CavityBall::init(){
  CavityBall* check = ballMap->value(zi * width * height + yi * width + xi);

  //cout << xi << "," << yi << "," << zi << "  : " << ballMap->mapSize() << "  check: " << check << endl;
  if(ballMap->value(zi * width * height + yi * width + xi))
     return;

  //  makeBall();
  // check the penalty for all of the volume points;
  pos origin(xi, yi, zi);
  float penalty = 0;
  float tp = 0;
  bool inBound = true;
  bool tooHigh = false;
  for(vector<pos>::iterator it=volume.begin(); it != volume.end(); ++it){
    pos cp = origin + (*it);
    //    cout << origin << " + " << (*it) << " = " << cp << endl;
    if(image->point(tp, cp, wi)){
      penalty += tp;
      if(tp > maxPenalty){
	tooHigh = true;
	break;
      }
    }else{
      inBound = false;
      break;
    }
    if( ballMap->value(oset(cp)) ){
      inBound = false;
      break;
    }
  }
  //cout << "End of looking for an origin for the ball: penalty : " << penalty << endl;
  // if we end up out of bounds, return without doing anything.
  if(!inBound || tooHigh){
    //    cout << "!inBound || tooHight" << endl;
    return;
  }
  
  // divide penalty by half the size of the surface.
  // Any movement in a specified direction will involve less than
  // half the points on the surface. (Since we have a tight wall, maybe quite
  // a bit less. 

  // if(( penalty / float(surface.size()/2) ) > maxDirPenalty){
  //   return;
  // }

  //  cout << "penalty / (surface.size() / 2) = " << penalty / float(surface.size()/2) << endl;
  // in this case, we can assign all the volume positions to this address.
  // set the directional penalty to be 0 at this point. That's probably a
  // bit silly since there is nothing to say that we will start in the middle
  // of a region.
  for(vector<pos>::iterator it=volume.begin(); it != volume.end(); ++it){
    pos cp = (*it) + origin;
    //    cout << "inserting pos into map: " << cp << endl;
    ballMap->insert(oset(cp), this);
    members.push_back(oset(cp));
  }
  //  dirPenalty dPenalty;
  expand(origin, pos(0, 0, 0));
  printBallStats();
}

void CavityBall::makeBall(){
  // Equations for the surface positions of a sphere;
  
  // The circumference of a circle of radius 1 in two dimensions can be defined as
  // x = cos(a)
  // y = sin(a)
  // where angle a ranges from -a to +a
  // in the case of a sphere, we also have to consider a second angle
  // then we can maybe say
  // y = sin(a)
  // x = cos(a) * cos(b)
  // z = cos(a) * sin(b)
  // This does seem to work (conceptually in my mind) angle a is in the y plane 
  // and angle b is in the z plane.
  // 
  // We need to use an appropriate angle to cover all the surface voxels.
  // Since the angle is given in radians, and angle of 1 will give a arc-length
  // of r (the radius). This means that an angle of 1/r will give an arc-length
  // of 1. Hence simply use angle increments of 1/r and fill up the structure.

  // make the angles (note that this will fail if xr != yr.
  // we could just say, whichever of them is smaller.. (I have a nagging feeling though
  // that there is something a little bit mathematically incorrect about all of this).
  int maxR = xRadius;
  maxR = maxR < yRadius ? yRadius : maxR;
  maxR = maxR < zRadius ? zRadius : maxR;
  vector<float> a;
  vector<float> b;
  for(float angle=-M_PI; angle <= M_PI; angle += 1.0 / float(maxR) ){
    a.push_back(angle);
    b.push_back(angle);
  }
  //  for(float angle=-M_PI; angle <= M_PI; angle += 1.0 / float(zRadius) )
  // b.push_back(angle);
  
  //cout << "init size of a " << a.size() << "  b " << b.size() << endl; 
  
  // Use a set to make the surface members.
  set<pos> tSurface;
  for(uint i=0; i < a.size(); ++i){
    for(uint j=0; j < b.size(); ++j){
      tSurface.insert( pos(xRadius * cosf(a[i]) * cosf(b[j]), 
			   yRadius * sinf(a[i]), 
			   zRadius * cosf(a[i]) * sinf(b[j]) ) );
    }
  }
  // Then we need to make up the volume of the ball. There are anumber of ways we could do
  // this, but I would suggest using a recursive flood fill starting at pos 0,0. 
  
  fillBall(tSurface);
}
  
void CavityBall::fillBall(set<pos>& tsurf){
  VolumeMask* mask = makeMask(tsurf);
  // and then flood the ball from the inside
  // starting from the central position
  //mask->printMask();
  fillMask(mask, xRadius + 1, yRadius + 1, zRadius + 1);
  //mask->printMask();

  // Then carefully fill the volume vector up.
  for(int z=0; z < zRadius * 2 + 3; ++z){
    for(int y=0; y < yRadius * 2 + 3; ++y){
      for(int x=0; x < xRadius * 2 + 3; ++x){
	if(mask->mask(x, y, z)){
	  volume.push_back(pos( x-(xRadius+1), y-(yRadius+1), z-(zRadius+1) ));
	  if( isSurface(mask, x, y, z) )
	    surface.push_back(pos( x-(xRadius+1), y-(yRadius+1), z-(zRadius+1) ));
	}
      }
    }
  }
  delete mask;
  //  mask = makeMask(surface);
  //mask->printMask();
}

VolumeMask* CavityBall::makeMask(set<pos>& tsurf){
  VolumeMask* mask = new VolumeMask((unsigned long)(xRadius * 2 + 3),
				    (unsigned long)(yRadius * 2 + 3),
				    (unsigned long)(zRadius * 2 + 3));

  // go through the surface points and assign the mask positions.
  // carefully (x + xRadius + 1) etc.
  for(set<pos>::iterator it=tsurf.begin(); it !=tsurf.end(); ++it)
    mask->setMask(true,
		  it->x + xRadius + 1,
		  it->y + yRadius + 1,
		  it->z + zRadius + 1);
  return(mask);
}

VolumeMask* CavityBall::makeMask(vector<pos>& tsurf){
  VolumeMask* mask = new VolumeMask((unsigned long)(xRadius * 2 + 3),
				    (unsigned long)(yRadius * 2 + 3),
				    (unsigned long)(zRadius * 2 + 3));

  // go through the surface points and assign the mask positions.
  // carefully (x + xRadius + 1) etc.
  for(vector<pos>::iterator it=tsurf.begin(); it !=tsurf.end(); ++it)
    mask->setMask(true,
		  it->x + xRadius + 1,
		  it->y + yRadius + 1,
		  it->z + zRadius + 1);
  return(mask);
}

void CavityBall::fillMask(VolumeMask* mask, int x, int y, int z){
  if(mask->mask(x, y, z))
    return;

  if(x < 0 || y < 0 || z < 0){
    cerr << "CavityBall::fillMask negative coordinate : " << x << "," << y << "," << z << endl;
    return;
  }
  if(x > xRadius * 2 + 2 || y > yRadius * 2 + 2 || z > zRadius * 2 + 2){
    cerr << "CavityBall::fillMask excessive coordinates : " << x << "," << y << "," << z << endl;
    return;
  }
  mask->setMask(true, x, y, z);
  fillMask(mask, x-1, y, z);
  fillMask(mask, x+1, y, z);
  fillMask(mask, x, y-1, z);
  fillMask(mask, x, y+1, z);
  fillMask(mask, x, y, z-1);
  fillMask(mask, x, y, z+1);
}

void CavityBall::expand(pos p, pos dp){
  //cout << "expand pos : " << endl; //<< dp << "\t" << p << " --> " << p + dp << endl;
  p = p + dp;
  if(dp.x || dp.y || dp.z){
    // evaluate the position here, and decide whether to return or not
    // if any new surface point already belongs to an old one return
    // if mean of new points is larger than penalty, then return
    // if directionalyPenalty is larger than maxDirPenalty then return
    // In any case, add all points that are not already claimed
    float penalty = 0;
    int count = 0;
    bool doReturn = false;
    float v;
    pos cp;
    //    map<o_set, CavityBall*>::iterator bmIt;
    //vector<float> newPoints;
    for(vector<pos>::iterator it=surface.begin(); it != surface.end(); ++it){
      cp = p + *it;
      //cout << cp << endl;
      if(cp.x < 0 || cp.y < 0 || cp.z < 0 ||
	 cp.x >= width || cp.y >= height || cp.z >= depth){
	doReturn = true;
	continue;
      }
      // Next check should not be necessary.. ?? 
      if(!image->point(v, cp, wi)){  // likely to be out of bounds. We don't want to go there
	cerr << "point out of bounds? " << cp << endl;
	doReturn = true;
	continue;
      }
      //cout << "\t" << v << endl;
      if(v > maxPenalty){
	//cout << "value above maxPenalty : " << v << " > " << maxPenalty << "  : " << cp << endl;
	doReturn = true;
	continue;
      }
      CavityBall* ball = ballMap->value(oset( cp ));
      //cout << "\tball: " << ball << endl;
      if(ball){
	if(ball != this){
	  doReturn = true;
	  //	  cout << "neighbouring another ball, don't continue" << endl;
	}
	continue;
      }
      //      cout << "\tinserting" << endl;
      ballMap->insert(oset(cp), this);
      members.push_back( oset(cp) );
      //newPoints.push_back(v);
      penalty += v;
      ++count;
    }
    //    cout << "\tcount : " << count << endl;
    if(!count)
      return;

    // sort(newPoints.begin(), newPoints.end());
    // unsigned index = (unsigned int)( float(newPoints.size()-1) * 1.0 );
    // dirP.addPenalty( (penalty / (float)count), dp.x, dp.y, dp.z);
    // // cout << "At point : " << p << " dirPenalty : "  
    // // 	 << dirP.penalty(dp.x, dp.y, dp.z)  << "  penalty : " 
    // // 	 << penalty << " / " << count << " = " << penalty / (float)count << endl;
    // if(newPoints[index] > maxPenalty || dirP.penalty(dp.x, dp.y, dp.z) > maxDirPenalty)
    //   doReturn = true;
    // //    if( (penalty / (float)count) > maxPenalty || dirP.penalty(dp.x, dp.y, dp.z) > maxDirPenalty)
    // // doReturn = true;
    if(doReturn){
      //      cout << "Returning after expanding to " << members.size() << "  map : " << ballMap->mapSize() << "  pos: " << cp << endl;
      return;
    }
  }
  //  cout << "\texpanding daughters: " << endl;
  for(int dx=-1; dx < 2; dx += 2){
    for(int dy=-1; dy < 2; dy += 2){
      for(int dz=-1; dz < 2; dz += 2){
	//cout << "calling expand : " << dx << "," << dy << "," << dz << " + " << p << endl;
	++recurseCount;
	if(members.size() > 1500000){
	  cout << "Returning since members too big: " << members.size() << "  recurseCount " << recurseCount << endl;
	  return;
	}
	expand(p, pos(dx, dy, dz));
      }
    }
  }
  //  cout << "Finished expansion" << endl;
  return;
}

// This is a strict surface check. If any of the closest neighbours are
// false then it is a surface member by default. This is important.
bool CavityBall::isSurface(VolumeMask* mask, int x, int y, int z){
  for(int dx=-1; dx < 2; dx += 2){
    for(int dy=-1; dy < 2; dy += 2){
      for(int dz=-1; dz < 2; dz += 2){
	if( !(mask->mask(x+dx, y+dy, z+dz) ) )
	    return(true);
      }
    }
  }
  return(false);
}
