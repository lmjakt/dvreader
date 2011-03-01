#include "CellTracer.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

CellTracer::CellTracer(Space* pointSpace)
{
  space=pointSpace;
}

CellTracer::~CellTracer()
{
  // ?? Cell Tracer doesn't keep ownership of the mask.
}

unsigned char* CellTracer::makeCellMask(Perimeter& nucleus, vector<Point*>& points, int max_d, 
					int& mx, int& my, int& mw, int& mh)
{
  maxD = max_d;
  globalWidth = nucleus.g_width();
  initMask(nucleus, points);
  mx = mask_x;
  my = mask_y;
  mw = maskWidth;
  mh = maskHeight;
  floodExterior(0, 0);
  return(mask);
}

void CellTracer::initMask(Perimeter& nucleus, vector<Point*>& points)
{
  int min_x = nucleus.xmin();
  int min_y = nucleus.ymin();
  int max_x = nucleus.xmax();
  int max_y = nucleus.ymax();
  
  // then go through the points and find the min and max positions.
  for(unsigned int i=0; i < points.size(); ++i){
    min_x = min_x > points[i]->x ? points[i]->x : min_x;
    min_y = min_y > points[i]->y ? points[i]->y : min_y;
    max_x = max_x < points[i]->x ? points[i]->x : max_x;
    max_y = max_y < points[i]->y ? points[i]->y : max_y;
  }
  mask_x = min_x - 2;
  mask_y = min_y - 2;

  maskWidth = 5 + max_x - min_x;
  maskHeight = 5 + max_y - min_y;

  mask = new unsigned char[ maskWidth * maskHeight ];
  memset((void*)mask, 0, sizeof(unsigned char) * maskWidth * maskHeight);

  for(unsigned int i=0; i < points.size(); ++i)
    mask[ maskWidth * (points[i]->y - mask_y) + points[i]->x - mask_x ] = node;

  int x, y;
  for(unsigned int i=0; i < nucleus.length(); ++i){
    nucleus.pos(i, x, y);
    if(x-mask_x <= 0 || y-mask_y <= 0 || x-mask_x >= maskWidth || y-mask_y >= maskHeight){
      cout << "initMask, nucleus position out of bounds" << endl;
      continue;
    }
    mask[ maskWidth * (y - mask_y) + x - mask_x ] |= nuc;
  }
  drawDaughterConnections(points);
}

void CellTracer::drawDaughterConnections(vector<Point*>& points)
{
  for(unsigned int i=0; i < points.size(); ++i){
    if(!points[i]->daughters.size()){
      drawOrphanConnections(points[i]);
      continue;
    }
    drawDaughterConnections(points[i]);
    //    drawOrphanConnections(points[i]);
  }
}

void CellTracer::drawDaughterConnections(Point* point)
{
  for(set<Point*>::iterator it=point->daughters.begin(); it != point->daughters.end(); ++it)
    drawLine(point->x, point->y, (*it)->x, (*it)->y);
}

// opposite of an orphan, has no daughters, but can't think of word
void CellTracer::drawOrphanConnections(Point* point)
{
  vector<Point*> npoints = neighbors(point);
  for(uint i=0; i < npoints.size(); ++i)   // draw only two connections
    drawLine(point->x, point->y, npoints[i]->x, npoints[i]->y);
}

// and the troublesome function..
// neighbor should have distance less than max_d and should have
// 
vector<Point*> CellTracer::neighbors(Point* point)
{
  multimap<int, Point*> npoints;
  int msq = maxD * maxD;
  int radius = maxD;
  while(radius <= maxD && npoints.size() < 2){
    npoints.clear();  // otherwise difficult..
    vector<Point*> pp = space->plane_points( point->x, point->y, radius );
    for(unsigned int i=0; i < pp.size(); ++i){
      if(pp[i] != point && 
	 !pp[i]->daughters.count(point) &&
	 //	 !pp[i]->daughters.size() &&
	 pp[i]->perimeter_id == point->perimeter_id &&
	 space->sq_distance( point, pp[i] ) <= msq)
	npoints.insert(make_pair(space->sq_distance(point, pp[i]),  pp[i]));
    }
    ++radius;
  }
  vector<Point*> n;
  for(multimap<int, Point*>::iterator it=npoints.begin(); it != npoints.end(); ++it){
    n.push_back((*it).second);
  }
  return(n);
}

void CellTracer::drawLine(int gx1, int gy1, int gx2, int gy2)
{
  int x1 = gx1 - mask_x;
  int x2 = gx2 - mask_x;
  int y1 = gy1 - mask_y;
  int y2 = gy2 - mask_y;
  
  int dx = x2 - x1;
  int dy = y2 - y1;
  int stepNo = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
  for(int i=0; i < stepNo; ++i){
    int lx = x1 + (i * dx)/stepNo;
    int ly = y1 + (i * dy)/stepNo;
    if(lx <= 0 || lx >= maskWidth || ly <= 0 || ly >= maskHeight){
      cout << "drawLine : trying to draw out of bounds : " << endl;
      continue;
    }
    mask[ ly * maskWidth + lx ] |= edge;
  }
}

void CellTracer::floodExterior(int x, int y)
{
  if(x < 0 || y < 0 || x >= maskWidth || y >= maskHeight)
    return;
  if(mask[y * maskWidth + x])
    return;
  mask[y * maskWidth + x] |= outside;
  
  floodExterior(x + 1, y);
  floodExterior(x - 1, y);
  floodExterior(x, y + 1);
  floodExterior(x, y - 1);
}
