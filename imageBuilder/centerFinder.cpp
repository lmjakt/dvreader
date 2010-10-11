#include "centerFinder.h"
#include <string.h>
#include <algorithm>
#include <math.h>

using namespace std;

CenterFinder::CenterFinder(float* img, int width, int height, vector<int> radiuses)
{
  image = 0;
  if(width <= 0 || height <= 0)
    return;       // since img = 0 && circles has no size should be safen
  sort(radiuses.begin(), radiuses.end());
  for(unsigned int i=0; i < radiuses.size(); ++i){
    if(radiuses[i] > 0 && (1 + 2 *radiuses[i]) < width && (1 + 2 * radiuses[i] < height))
      circles.push_back( cf_circle(0, 0, radiuses[i]));
  }
  if(circles.size())
    image = img;
  imWidth = width;
  imHeight = height;
}

vector<cf_circle> CenterFinder::findCenter(){
  // do something magical, and then ..
  for(unsigned int i=0; i < circles.size(); ++i){
    cf_circle* circle = &circles[i];
    char* disc = makeDisc(circle->radius);
    float maxSum = 0;
    for(int dy=circle->radius; dy < (imHeight - circle->radius); ++dy){
      for(int dx=circle->radius; dx < (imWidth - circle->radius); ++dx){
	float sum = scanDisc(disc, circle->radius, dx, dy);
	if(sum > maxSum){
	  maxSum = sum;
	  circle->x = dx;
	  circle->y = dy;
	}
      }
    }
    delete []disc;
  }
  return(circles);
}

// 0 = outside, 1 = inside, 2 = 
char* CenterFinder::makeDisc(int radius){
  int d = radius * 2 + 1;
  char* disc = new char[d * d];
  memset((void*)disc, 0, sizeof(char) * d * d);
  // do some funny things..
  char* dptr = disc;
  float rf = radius;
  for(int dy=-radius; dy <= radius; ++dy){
    for(int dx=-radius; dx <= radius; ++dx){
      float df = sqrt( (dx * dx) + (dy * dy) );
      if(df > rf)
	continue;
      if(rf - df <= 1.0){
	*dptr = 2;
	continue;
      }
      *dptr = 1;
      ++dptr;
    }
  }
  return(disc);
}

float CenterFinder::scanDisc(char* disc, int radius, int xo, int yo){
  if(xo < radius || yo < radius)
    return(0);
  if((xo + radius) >= imWidth || (yo + radius) >= imHeight)
    return(0);
  char* dptr = disc;
  float sum = 0;
  for(int dy=-radius; dy <= radius; ++dy){
    float* imptr = image + (yo + dy - radius) * imWidth + xo - radius;
    for(int dx=-radius; dx <= radius; ++dx){
      if(!(*dptr))
	continue;
      sum += (*imptr);
      ++imptr;
      ++dptr;
    }
  }
  return(sum);
}
