#include "idMap.h"
#include "frameStack.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>

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
  if(x < x_pos){
    w += (x - x_pos);
    x = x_pos;
  }
  if(y < y_pos){
    h += (y - y_pos);
    y = y_pos;
  }
  //x = x < x_pos ? x_pos : x;
  //y = y < y_pos ? y_pos : y;
  w = x + w > (x_pos + width) ? (x_pos + width) - x : w;
  h = y + h > (y_pos + height) ? (y_pos + height) - y : h;

  for(int yp=y; yp < (y + h); ++yp){
    for(int xp=x; xp < (x + w); ++xp){
      map[ (yp-y_pos) * width + (xp-x_pos) ] |= id;
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
     
  return( map[ (y - y_pos) * width + (x - x_pos)] & id );
}

ulong IdMap::id(int x, int y){
  return( map[ (y-y_pos) * width + (x-x_pos) ] );
}

// returns the number of assigned ids at x, y
int IdMap::count(int x, int y){
  if(x < x_pos || x >= x_pos + width)
    return(0);
  if(y_pos >= y || y >= y_pos + height)
    return(0);
  int count = 0;
  ulong check = 1;
  ulong target = map[ (y-y_pos) * width + (x-x_pos) ];
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

void IdMap::setContribMaps(std::map<ulong, FrameStack*> idMap, float pixelFactor){
  for(std::map<ulong, FrameStack*>::iterator it = idMap.begin();
      it != idMap.end(); ++it){
    ulong f_id = it->first;
    FrameStack* frame = it->second;
    int fx = frame->left();
    int fy = frame->bottom();
    int fw = frame->p_width();
    int fh = frame->p_height();
    // BIG BUG: the id() function assumes that the x_pos and y_pos are both 0
    // and that all numbers given to it are positive. Hence we have to make sure
    // that we don't put in any bad numbers here.
    if(fx < x_pos){
      fw += (fx - x_pos);  // reduces the width.
      fx = x_pos;
    }
    if(fy < y_pos){
      fh += (fy - y_pos);
      fy = y_pos;
    }
    fw = (fx + fw) <= (x_pos + width) ? fw : (x_pos + width) - fx;
    fh = (fy + fw) <= (y_pos + height) ? fh : (y_pos + height) - fy;

    float* cMap = new float[ frame->p_width() * frame->p_height() ];
    memset((void*)cMap, 0, sizeof(float)* frame->p_width() * frame->p_height());
    for(int y=fy; y < fy + fh; ++y){
      for(int x=fx; x < fx + fw; ++x){
	ulong m_id = id(x, y);
	if(!m_id)
	  continue;
	if(m_id == f_id){
	  cMap[ (y - frame->bottom()) * frame->p_width() + (x - frame->left()) ] = 1.0;
	  continue;
	}
	if(!count(f_id, x, y))
	  continue;
	// If we are here, there are multiple identities assigned.
	float weight = calculateWeights(x, y, frame, frameStacks(m_id, idMap), pixelFactor);
	cMap[ (y - frame->bottom()) * frame->p_width() + (x - frame->left()) ] = weight;
      }
    }
    // do something to set the contribMap to the appropriate frameStack here.
    frame->setContribMap(cMap);
  }
}

std::vector<ulong> IdMap::components(ulong ul){
  std::vector<ulong> comp;
  comp.reserve(30);
  ulong check = 1;
  while(check){
    if(check & ul)
      comp.push_back(check);
    check <<= 1;
  }
  return(comp);
}

std::vector<FrameStack*> IdMap::frameStacks(ulong ul, std::map<ulong, FrameStack*> idMap){
  std::vector<ulong> comp = components(ul);
  std::vector<FrameStack*> stacks;
  stacks.reserve(4);
  for(uint i=0; i < comp.size(); ++i){
    //    std::cout << "IdMap::frameStacks() ul : " << ul << "  i: " << i << " --> " << comp[i]
    //	      << "  idMap.count: " << idMap.count(comp[i]) << std::endl;
    if(idMap.count(comp[i]))
      stacks.push_back(idMap[comp[i]]);
  }
  return(stacks);
}

// This is inherently inefficient as this function will be called n times (up to 4 times)
// for every overlapping pixel. I'm doing this as to reduce memory use. Since it will
// only be done once, it may not matter too much.
float IdMap::calculateWeights(int x, int y, FrameStack* frame, 
			      std::vector<FrameStack*> stacks, float pixelFactor){
  std::vector<float> weights(stacks.size());
  std::vector<int> border_distances(stacks.size());
  int maxBorderDistance = 0;
  for(uint i=0; i < stacks.size(); ++i){
    border_distances[i] = stacks[i]->nearestBorderGlobal(x, y);
    maxBorderDistance = border_distances[i] > maxBorderDistance ? border_distances[i] : maxBorderDistance;
  }
  // override pixelFactor with MaxBorderDistance
  pixelFactor = maxBorderDistance ? (float)maxBorderDistance : pixelFactor;
  int offset = -1;
  float w_sum = 0;
  for(uint i=0; i < stacks.size(); ++i){
    if(stacks[i] == frame)
      offset = i;
    int border_distance = stacks[i]->nearestBorderGlobal(x, y);
    weights[i] = border_distance > (int)pixelFactor ? 1.0 : (float)(1 + border_distance) / (1.00 + pixelFactor);
    weights[i] = weights[i] < 0 ? 0 : weights[i];
    w_sum += weights[i];
  }
  // float w_sum = 0;
  // for(uint i=0; i < weights.size(); ++i)
  //   w_sum += weights[i];
  if(!w_sum){
    std::cerr << "idMap::calculateWeights: w_sum is 0" << std::endl;
    std::cerr << "\tx,y: " << x << "," << y << " offset : " << offset
	      << "\tstacks.size() " << stacks.size() << std::endl;
    exit(1);
  }
  for(uint i=0; i < weights.size(); ++i)
    weights[i] /= w_sum;
  if(offset < 0){
    std::cerr << "idMap::calculateWeights: offset not defined returning 0 weighting" << std::endl;
    return(0);
  }
  return(weights[offset]);
}
