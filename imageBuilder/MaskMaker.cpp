#include "MaskMaker.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;

MaskMaker::MaskMaker(QPoint mask_pos, int w, int h)
{
  mask = maskPicture = maskPictureBuffer = 0;
  origin = mask_pos;
  width = abs(w);
  height = abs(h);
  if(!width || !height)
    return;
  mask = new unsigned char[width * height];
  maskPicture = new unsigned char[ width * height * 4 ];
  maskPictureBuffer = new unsigned char[ width * height * 4 ];
  memset((void*)mask, 0, sizeof(unsigned char) * width * height);
  memset((void*)maskPicture, 40, sizeof(unsigned char) * 4 * width * height);
  memset((void*)maskPictureBuffer, 40, sizeof(unsigned char) * 4 * width * height);

  red = green = blue = alpha = 255;

}

MaskMaker::~MaskMaker()
{
  delete []mask;
  delete []maskPicture;
  delete []maskPictureBuffer;
}

void MaskMaker::setPerimeter(vector<QPoint> p, bool global)
{
  if(global){
    for(uint i=0; i < p.size(); ++i)
      globalToLocal(p[i]);
  }
  // draw lines between points.. 
  unsigned int p1 = 0;
  while(p1 < p.size()){
    cout << p1 << " : " << p[p1].x() << "," << p[p1].y() << endl;
    if(checkPoint(p[p1]))
      break;
    ++p1;
  }
  unsigned int p2 = p1 + 1;
  while(p2 < p.size()){
    if(checkPoint(p[p2]))
      break;
    ++p2;
  }
  if(p1 >= p.size() || p2 >= p.size())
    return;
  unsigned int o = p1; // the origin position.
  
  perimeter.resize(0);

  while(p2 < p.size()){
    if(!checkPoint(p[2])){
      ++p2;
      continue;
    }
    vector<QPoint> points = drawLine(p[p1], p[p2], maskPicture);
    perimeter.insert(perimeter.end(), points.begin(), points.end());
    p1 = p2;
    ++p2;
  }
  // at this point p1 is the last known good point that was drawn to. Hence..
  vector<QPoint> points = drawLine(p[p1], p[o], maskPicture);
  perimeter.insert(perimeter.end(), points.begin(), points.end());
  setMask(perimeter, PERIMETER, false);
  mempcpy((void*)maskPictureBuffer, (void*)maskPicture, sizeof(unsigned char) * 4 * width * height);
  
}

void MaskMaker::startSegment(QPoint p, bool global)
{
  if(global)
    globalToLocal(p);
  if(checkPoint(p)){
    currentSegment.push_back(p);
    currentSegmentPoints.push_back(p);
  }
}

void MaskMaker::addSegmentPoint(QPoint p, bool global)
{
  if(!currentSegment.size()){
    startSegment(p, global);
    return;
  }
  if(global)
    globalToLocal(p);
  if(!checkPoint(p))
    return;
  vector<QPoint> points = drawLine(currentSegmentPoints.back(), p, maskPictureBuffer);
  currentSegmentPoints.insert(currentSegmentPoints.end(), points.begin(), points.end());
  currentSegment.push_back(p);
}

void MaskMaker::endSegment(QPoint p, bool global)
{
  if(global)
    globalToLocal(p);
  if(checkPoint(p)){
    vector<QPoint> points = drawLine(currentSegmentPoints.back(), p, maskPictureBuffer);
    currentSegmentPoints.insert(currentSegmentPoints.end(), points.begin(), points.end());
  }
  // check segment..
  if(checkSegment(currentSegmentPoints)){
    drawPoints(currentSegmentPoints, maskPicture);
    setMask(currentSegmentPoints, SEGMENT, true);
    borderSegments.push_back( currentSegmentPoints );
  }
  memcpy((void*)maskPictureBuffer, (void*)maskPicture, sizeof(unsigned char) * 4 * width * height);
  currentSegment.resize(0);
  currentSegmentPoints.resize(0);
}

void MaskMaker::setMaskColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  red = r;
  green = g;
  blue = b;
  alpha = a;
}

unsigned char* MaskMaker::maskImage(QPoint& pos, int& w, int& h)
{
  pos = origin;
  w = width;
  h = height;
  if(!currentSegment.size())
    return(maskPicture);
  return(maskPictureBuffer);
}

// draws a line from the 
vector<QPoint> MaskMaker::drawLine(QPoint& p1, QPoint& p2, unsigned char* m)
{
  int dx = abs(p2.x() - p1.x());
  int dy = abs(p2.y() - p1.y());
  int nstep = dx > dy ? dx : dy;
  
  vector<QPoint> points;

  for(int i=1; i <= nstep; ++i){
    int x = p1.x() + (i * dx) / nstep;
    int y = p1.y() + (i * dy) / nstep;
    int o = 4 * (y * width + x);
    m[ o ] = red;
    m[ o + 1 ] = green;
    m[ o + 2 ] = blue;
    m[ o + 3 ] = alpha;
    points.push_back(QPoint(x, y));
  }
  return(points);
}

void MaskMaker::drawPoints(vector<QPoint>& points, unsigned char* m)
{
  for(uint i=0; i < points.size(); ++i){
    int n = 3 * (points[i].y() * width + points[i].x());
    m[n] = red;
    m[n+1] = green;
    m[n+2] = blue;
    m[n+3] = alpha;
  }
}

// For a segment to be ok, it must cross the perimeter at least 2 times. 
// points will be redefined from the first crossing point to the last one
// and this will be used for the mask and also the drawing.. 
bool MaskMaker::checkSegment(vector<QPoint>& points)
{
  int end, begin;
  end = begin = -1;
  for(uint i=0; i < points.size(); ++i){
    if(maskValue(points[i]) == PERIMETER){
      begin = i;
      break;
    }
  }
  if(begin == -1)
    return(false);
  for(uint i=(points.size() - 1); i < points.size(); --i){  // unsigned will roll over below 0
    if(maskValue(points[i]) == PERIMETER){
      end = i;
      break;
    }
  }
  if(begin >= end)
    return(false);
  vector<QPoint> okPoints;
  for(int i=begin; i <= end; ++i)
    okPoints.push_back(points[i]);
  return(true);
}

void MaskMaker::setMask(vector<QPoint>& points, unsigned char v, bool setEnds)
{
  if(!points.size())
    return;
  for(uint i=0; i < points.size(); ++i)
    mask[ points[i].y() * width + points[i].x() ] |= v;
  if(setEnds){
    mask[ points[0].y() * width + points[0].x() ] |= BEGIN;
    mask[ points.back().y() * width + points.back().x() ] |= END;
  }
}
