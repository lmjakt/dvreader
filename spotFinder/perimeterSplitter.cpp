#include "perimeterSplitter.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

const int outside_id = 1;
const int border_id = 2;
const int within_id = 4;
const int split_border_id = 8;

PerimeterSplitter::PerimeterSplitter()
{
  mask = 0;
  globalWidth = maskWidth = maskHeight = 0;
  mask_x = mask_y = 0;
}

PerimeterSplitter::~PerimeterSplitter()
{
  if(mask)
    delete []mask;
}

void PerimeterSplitter::splitPerimeter(Perimeter& per, vector<vector<int> >& splitLines)
{
  initMask(per);
  for(unsigned int i=0; i < splitLines.size(); ++i)
    addLineToMask(splitLines[i]);
  finaliseMask();
}

unsigned int* PerimeterSplitter::perimeterMask(int& mw, int& mh)
{
  mw = maskWidth;
  mh = maskHeight;
  return(mask);
}

map<unsigned int, vector<int> > PerimeterSplitter::newPerimeters()
{
  map<unsigned int, vector<int> > per;
  for(unsigned int i=0; i < areaIds.size(); ++i)
    per.insert(make_pair(areaIds[i], tracePerimeter(areaIds[i])));
  return(per);
}

vector<vector<int> > PerimeterSplitter::newPerimetersV()
{
  map<unsigned int, vector<int> > per_m = newPerimeters();
  vector<vector<int> > per_v;
  per_v.reserve(per_m.size());
  for(map<unsigned int, vector<int> >::iterator it=per_m.begin(); it != per_m.end(); ++it)
    per_v.push_back( (*it).second );
  return(per_v);
}

void PerimeterSplitter::initMask(Perimeter& per)
{
  if(mask)
    delete []mask;
  globalWidth = per.g_width();
  mask_x = per.xmin() - 1;
  mask_y = per.ymin() - 1;
  maskWidth = 2 + (per.xmax() - mask_x);      // +2 because we want an empty first rows and columns
  maskHeight = 2 + (per.ymax() - mask_y);
  
  if(maskHeight <= 2 || maskWidth <= 2){
    cerr << "PerimeterSplitter::initMask, maskHeight or maskWidth too low : " << maskWidth << "," << maskHeight << endl;
    return;
  }
  mask = new unsigned int[maskWidth * maskHeight];
  memset((void*)mask, 0, sizeof(unsigned int) * maskWidth * maskHeight);
  int x,y;
  for(unsigned int i=0; i < per.length(); ++i){
    per.pos(i, x, y);
    mask[ x - mask_x + maskWidth * (y - mask_y) ] = border_id;
  }
  // flood fill the outside with outside_id
  flood_or_mask(0, 0, (border_id + outside_id), outside_id, false);
  if(!per.length())
    return;
  // then the inside of the perimeter using within_id (x,y) should be the last point on the perimeter
  flood_or_mask(x-mask_x, y-mask_y, (outside_id + within_id), within_id, false);
}

// points are in global coordinates..
// uses mask, so mask has to be set up correctly first. Makes a boundary from the first to
// the second intersection point.
/*
  The way the split line gets created, it has to start and end outside of the perimeter.
  This is due to both the border and the split line allowing open diagonal lines, such
  that they can cross each other. This can be fixed by ensuring that the split lines
  contain only filled diagonals; (ultimately that is what I should do.. but need to think
  about how to do it).
*/
void PerimeterSplitter::addLineToMask(vector<int>& points)
{
  int x, y;
  //  int beg, end;
  //beg = end = -1;
  //int p = 0;
  //  unsigned int check_id = border_id + split_border_id;
  // for(unsigned int i=0; i < points.size(); ++i){
  //   if(beg != -1 && end != -1)
  //     break;
  //   x = (points[i] % globalWidth) - mask_x;
  //   y = (points[i] / globalWidth) - mask_y;
  //   p = x + y * maskWidth;
  //   if(beg == -1 && mask[p] & (border_id + within_id)){
  //     beg = i;
  //     continue;
  //   }
  //   if(beg != -1 && mask[p] & (border_id + outside_id) ){
  //     end = i;
  //     break;
  //   }
  // }
  // if(beg == -1 || end == -1)
  //   return;
  for(unsigned int i=0; i < points.size(); ++i){
    x = (points[i] % globalWidth) - mask_x;
    y = (points[i] / globalWidth) - mask_y;
    mask[ x + y * maskWidth] |= split_border_id;
  }
}

// assign values to new subdivisions.. 
// this doesn't check for area_id overflow, 
void PerimeterSplitter::finaliseMask()
{
  areaIds.resize(0);
  unsigned int area_id = split_border_id * 2;
  unsigned int forbidden_value = outside_id + split_border_id;
  for(int y=0; y < maskHeight; ++y){
    for(int x=0; x < maskWidth; ++x){
      if(mask[ x + y * maskWidth] & within_id && !(mask[ x + y * maskWidth] & forbidden_value)){

	flood_or_mask(x, y, forbidden_value + area_id, area_id, true);
	areaIds.push_back(area_id);
	area_id *= 2;
	if(!area_id){
	  cerr << "PerimeterSplitter too many subdivisions, consider moving to 64 bit mask" << endl;
	  exit(1);
	}
      }
    }
  }
}

/// uses local parameters !!!
// set_value indicates whether we do = or |=
void PerimeterSplitter::flood_or_mask(int x, int y, unsigned int forbidden, unsigned int flood_value, bool set_value)
{
  if(x < 0 || x >= maskWidth)
    return;
  if(y < 0 || y >= maskHeight)
    return;
  if(mask[ y * maskWidth + x ] & forbidden)
    return;

  if(set_value){
      mask[ y * maskWidth + x ] = flood_value;
  }else{
    mask[ y * maskWidth + x ] |= flood_value;
  }
  
  flood_or_mask(x-1, y, forbidden, flood_value, set_value);
  flood_or_mask(x+1, y, forbidden, flood_value, set_value);
  flood_or_mask(x, y-1, forbidden, flood_value, set_value);
  flood_or_mask(x, y+1, forbidden, flood_value, set_value);
}

vector<int> PerimeterSplitter::tracePerimeter(unsigned int p_id)
{
  vector<int> points;
  // These arrays contain coordinates that allow a clockwise rotation around
  // the current point; with the starting point decided by the previous entry position
  // It's horrible to read and understand the flow.
  int xoffsets[] = {-1, 0, 1, 1, 1, 0, -1, -1};
  int yoffsets[] = {1, 1, 1, 0, -1, -1, -1, 0};     // this gives us a clockwise spin around a central position.. 
  int offset_offsets[] = {5, 6, 7, 0, 1, 2, 3, 4};  //    
  int offsetPos = 0;

  int x, y;
  x = y = -1;
  for(int i=0; i < (maskWidth * maskHeight); ++i){
    if(mask[i] == p_id){
      x = i % maskWidth;
      y = i / maskWidth;
      break;
    }
  }
  if(x == -1 || y == -1)
    return(points);

  bool tracing = true;
  int mask_origin = y * maskWidth + x;
  points.push_back( globalWidth * (y + mask_y) + x + mask_x );
  while(tracing){
    for(int i=0; i < 8; ++i){
      int op = (offsetPos + i) % 8;
      int nx = x + xoffsets[op];
      int ny = y + yoffsets[op];
      if( ny * maskWidth + nx == mask_origin ){
	tracing = false;
	break;
      }
      if(nx < 0 || nx >= maskWidth)
	continue;
      if(ny < 0 || ny >= maskHeight)
	continue;
      if(mask[ ny * maskWidth + nx ] == p_id){
	points.push_back( globalWidth * (y + mask_y) + x + mask_x );
	offsetPos = offset_offsets[op];
	x = nx;
	y = ny;
	break;
      }
    }
    if(points.size() < 2)    // in the case of a single point this can happen.
      break;
  }
  return(points);
}
