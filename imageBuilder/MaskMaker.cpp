#include "MaskMaker.h"
#include <QKeyEvent>
#include <QPointF>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <set>

using namespace std;

MaskMaker::MaskMaker(QPoint mask_pos, int w, int h, QObject* parent)
  : QObject(parent)
{
  perimeter_source = "";
  perimeter_id = -1;
  mask = maskPicture = 0;
  newMask(mask_pos, w, h);
  // origin = mask_pos;
  // width = abs(w);
  // height = abs(h);
  // if(!width || !height)
  //   return;
  // mask = new unsigned char[width * height];
  // maskPicture = new unsigned char[ width * height * 4 ];
  // //maskPictureBuffer = new unsigned char[ width * height * 4 ];
  // memset((void*)mask, 0, sizeof(unsigned char) * width * height);
  // memset((void*)maskPicture, 40, sizeof(unsigned char) * 4 * width * height);
  // //memset((void*)maskPictureBuffer, 40, sizeof(unsigned char) * 4 * width * height);

  red = green = blue = alpha = 255;

}

void MaskMaker::newMask(QPoint mask_pos, int w, int h)
{
  origin = mask_pos;
  width = abs(w);
  height = abs(h);
  if(!width || !height)
    return;
  if(mask) delete []mask;
  if(maskPicture) delete []maskPicture;
  mask = new unsigned char[width * height];
  maskPicture = new unsigned char[ width * height * 4 ];
  //maskPictureBuffer = new unsigned char[ width * height * 4 ];
  memset((void*)mask, 0, sizeof(unsigned char) * width * height);
  memset((void*)maskPicture, 40, sizeof(unsigned char) * 4 * width * height);
}

MaskMaker::~MaskMaker()
{
  delete []mask;
  delete []maskPicture;
  //delete []maskPictureBuffer;
}

void MaskMaker::setPerimeter(vector<QPoint> p, int id, QString source, bool global)
{
  perimeter_id = id;
  perimeter_source = source;
  setPerimeter(p, global, false);
}

void MaskMaker::setPerimeter(vector<QPoint> p, bool global, bool resetIDs)
{
  if(resetIDs){
    perimeter_id = -1;
    perimeter_source = "";
  }
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
    vector<QPoint> points = drawLine(p[p1], p[p2], maskPicture, true);
    perimeter.insert(perimeter.end(), points.begin(), points.end());
    p1 = p2;
    ++p2;
  }
  // at this point p1 is the last known good point that was drawn to. Hence..
  vector<QPoint> points = drawLine(p[p1], p[o], maskPicture, true);
  perimeter.insert(perimeter.end(), points.begin(), points.end());
  removeDuplicates(perimeter);  // it seems that drawLine can introduce duplicates.
  setMask(perimeter, PERIMETER, false);
  //mempcpy((void*)maskPictureBuffer, (void*)maskPicture, sizeof(unsigned char) * 4 * width * height);
  original_perimeter = perimeter;
}

void MaskMaker::setCellSource(QString source, int id)
{
  perimeter_id = id;
  perimeter_source = source;
}

int MaskMaker::per_id()
{
  return(perimeter_id);
}

QString MaskMaker::per_source()
{
  return(perimeter_source);
}

vector<QPoint> MaskMaker::getPerimeter(int& id, QString& source, bool global)
{
  id = perimeter_id;
  source = perimeter_source;
  if(!global)
    return(perimeter);
  vector<QPoint> gper(perimeter.size());
  for(unsigned int i=0; i < perimeter.size(); ++i)
    gper[i] = perimeter[i] + origin;
  return(gper);
}

void MaskMaker::startSegment(QPoint p, bool global)
{
  cout << "start : " << p.x() << "," << p.y() << " --> ";
  if(global)
    globalToLocal(p);
  if(checkPoint(p)){
    cout << p.x() << "," << p.y();
    currentSegment.push_back(p);
    currentSegmentPoints.push_back(p);
  }
  cout << endl;
}

void MaskMaker::addSegmentPoint(QPoint p, bool global)
{
  if(!currentSegment.size()){
    startSegment(p, global);
    return;
  }
  cout << "add  : " << p.x() << "," << p.y() << " --> ";
  if(global)
    globalToLocal(p);
  cout << p.x() << "," << p.y() << endl;
  if(!checkPoint(p)){
    return;
  }
  QPoint p1 = currentSegmentPoints.back();
  cout << "\t\tp1 : " << p1.x() << "," << p1.y() << endl;
  vector<QPoint> points = drawLine(p1, p, maskPicture, true);
  currentSegmentPoints.insert(currentSegmentPoints.end(), points.begin(), points.end());
  currentSegment.push_back(p);
}

void MaskMaker::endSegment(QPoint p, bool global)
{
  cout << "end : " << p.x() << "," << p.y() << "  --> ";
  if(!currentSegmentPoints.size())
    return;
  if(global)
    globalToLocal(p);
  cout << p.x() << "," << p.y() << endl; 
  if(checkPoint(p)){
    vector<QPoint> points = drawLine(currentSegmentPoints.back(), p, maskPicture, true);
    currentSegmentPoints.insert(currentSegmentPoints.end(), points.begin(), points.end());
  }
  // check segment..
  removeDuplicates(currentSegmentPoints);
  if(checkSegment(currentSegmentPoints)){
    //    drawPoints(currentSegmentPoints, maskPicture);
    setMask(currentSegmentPoints, SEGMENT, true);
    borderSegments.push_back( currentSegmentPoints );
  }
  drawFromMask();
  //  memcpy((void*)maskPictureBuffer, (void*)maskPicture, sizeof(unsigned char) * 4 * width * height);
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
  //  if(!currentSegment.size())
  return(maskPicture);
  //return(maskPictureBuffer);
}

void MaskMaker::keyPressed(QKeyEvent* e)
{
  int key = e->key();
  //  if(e->modifiers() == Qt::ControlModifier){   // maybe good to use without modifier key. 
  switch(key){
  case Qt::Key_M :
    mergeSegments();
    break;
  case Qt::Key_R :
    borderSegments.resize(0);             // otherwise no real point
    resetPerimeter(original_perimeter);   // this sets the mask and draws the picture.
    emit maskChanged();
    break;
  case Qt::Key_Left :
    emit increment(-1);
    break;
  case Qt::Key_Down :
    emit increment(-1);
    break;
  case Qt::Key_Right :
    emit increment(1);
    break;
  case Qt::Key_Up :
    emit increment(1);
    break;
  case Qt::Key_Return :
    emit perimeterModified();
    emit increment(1);
    break;  
  default :
    e->ignore();
  }
  return;
}

// draws a line from the 
vector<QPoint> MaskMaker::drawLine(QPoint& p1, QPoint& p2, unsigned char* m, bool filled)
{
  int dx = (p2.x() - p1.x());
  int dy = (p2.y() - p1.y());
  int nstep = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
  
  vector<QPoint> points;

  // special case if p1 == p2, just draw one point and return the point.
  if(p1 == p2){
    // do stupid stuff
    setRGBA(m + 4 * (p1.y() * width + p1.x()), red, green, blue, alpha);
    points.push_back(p1);
    return(points);
  }

  //cout << "\t\t\t draw : " << p1.x() << "," << p1.y() << " + " << dx << "," << dy << endl;
  int lx = p1.x();
  int ly = p1.y();  // last_x and last_y use in order to prevent holes in lines.
  for(int i=0; i <= nstep; ++i){
    int x = p1.x() + (i * dx) / nstep;
    int y = p1.y() + (i * dy) / nstep;
    if(filled && (x - lx) && (y - ly) ){
      setRGBA(m + 4 * (y * width + lx), red, green, blue, alpha);
      points.push_back(QPoint(lx, y));
    }
    setRGBA(m + 4 * (y * width + x), red, green, blue, alpha);
    points.push_back(QPoint(x, y));
    lx = x;
    ly = y;
  }
  return(points);
}

void MaskMaker::drawPoints(vector<QPoint>& points, unsigned char* m)
{
  for(uint i=0; i < points.size(); ++i){
    setRGBA(m + 4 * (points[i].y() * width + points[i].x()), red, green, blue, alpha);
    // int n = 4 * 
    // m[n] = red;
    // m[n+1] = green;
    // m[n+2] = blue;
    // m[n+3] = alpha;
  }
}

// For a segment to be ok, it must cross the perimeter at least 2 times. 
// points will be redefined from the first crossing point to the last one
// and this will be used for the mask and also the drawing.. 
bool MaskMaker::checkSegment(vector<QPoint>& points)
{
  cout << "checkSegment points.size() : " << points.size() << endl;
  int end, begin;
  end = begin = -1;
  for(uint i=0; i < points.size(); ++i){
    if(maskValue(points[i]) & PERIMETER){
      while( i < (points.size() - 1) && maskValue(points[i+1]) & PERIMETER)
	++i;
      begin = i;
      break;
    }
  }
  if(begin == -1)
    return(false);
  for(int i=(int)(points.size() - 1); i >= 0; --i){  // unsigned will roll over below 0
    if(maskValue(points[i]) & PERIMETER){
      while(i > 0 && maskValue(points[i-1]) & PERIMETER)     // the wall is thicker.. 
	--i;
      end = i;
      break;
    }
  }
  if(begin >= end)
    return(false);
  cout << "checkSegment " << begin << " -> " << end << endl;
  vector<QPoint> okPoints;
  unsigned int per_beg, per_end;
  if(!perimeterIndexAt(points[begin], per_beg) || !perimeterIndexAt(points[end], per_end)){
    cerr << "CheckSegment Unable to find perimeter index position" << endl;
    return(false);
  }
  int inc = 1;
  int b = begin;
  int e = end;
  // Everything fails if the user doesn't make a clockwise extension. But how to determine the clockwise nature of
  // the selection?? It's easy to do as long as the user doesn't select cross the 0 position. Otherwise everything
  // cannot be determined. One can look at the arc length between crossing points, but that won't always work.
  // I'm afraid that the only good way is to look at angles of various things, but not sure how to do if the
  // starting cell has a very strange shape. So for now the user has to make a clockwise selection.

  // IDEA : use the mid point between begin and end. Then take angle differences between successive points.
  // add up angles. That should work easily enough. (And one can make a simple function). My only concern
  // is the symmetry in sin and cos measurements. Need to study up my trigonometry.

  // no need to do angles. Just count ups and downs depending on location. if below mid point left = clockwise

  for(int i=b; i != e; i += inc)
    okPoints.push_back(points[i]);
  okPoints.push_back(points[e]);
  checkSegmentDirection(okPoints);
  points = okPoints;
  return(true);
}
// do in the simplest way. NOT the fastest way.
void MaskMaker::checkSegmentDirection(vector<QPoint>& points)
{
  if(points.size() < 3)
    return;
  // It's unlikely to be necessary to convert to float points
  // but consider a three point segment. If connected it would
  // need float representation to give the correct answer.
  vector<QPointF> fpoints(points.size());
  for(unsigned int i=0; i < points.size(); ++i)
    fpoints[i] = points[i];
  QPointF mid = (fpoints[0] + fpoints.back()) / 2;
  for(unsigned int i=0; i < fpoints.size(); ++i)
    fpoints[i] -= mid;

  // look at movements depending on position.
  // if y > 0, right movement is clockwise
  // if x > 0, downwards movement is clockwise
  float cwise_sum = 0;
  for(unsigned int i=1; i < fpoints.size(); ++i){
    float dx = (fpoints[i] - fpoints[i-1]).x();
    float dy = (fpoints[i] - fpoints[i-1]).y();

    float mx = fpoints[i-1].y() > 0 ? 1 : -1;
    float my = fpoints[i-1].x() > 0 ? -1 : 1;
    cwise_sum += (mx * dx) + (my * dy);
  }
  cout << "CheckSegmentDirection cwise_sum : " << cwise_sum << endl;
  if(cwise_sum > 0)
    return;
  // we need to reverse the points vector
  vector<QPoint> rpoints(points.size());
  for(unsigned int i=1; i <= points.size(); ++i)  // a reverse iterator would be more efficient, but uglier
    rpoints[ points.size() - i ] = points[i-1];
  points = rpoints;
}

void MaskMaker::mergeSegments()
{
  cout << "Merging the segments" << endl;
  // start at the first position of the first border Segment and trace the positions.
  if(!borderSegments.size())
    return;

  set<int> point_set;
  vector<QPoint> newPerimeter;
  vector<QPoint>& segment = borderSegments[0];
  bool complete = false;
  while(!complete){
    if(!addPoints(newPerimeter, segment, point_set))
      break;
    QPoint lastPoint = newPerimeter.back();
    if( (maskValue( lastPoint ) & BEGIN) && segmentFrom( lastPoint, segment ) )
      continue;
    unsigned int per_start=0;
    if( !perimeterIndexAt(lastPoint, per_start) )
      break;
    while(!complete){
      ++per_start;
      unsigned int i = per_start % perimeter.size();
      int off = perimeter[i].y() * width + perimeter[i].x();
      if(point_set.count(off)){
	complete = true;
	break;
      }
      if( (maskValue( perimeter[i] ) & BEGIN) && segmentFrom( perimeter[i], segment ) )
	break;  // goes back to adding points from segment.. 
      point_set.insert(off);
      newPerimeter.push_back( perimeter[i] );
      paintPoint(perimeter[i], 0, 0, 255, 255);
      cout << "per point added : " << i << endl;
    }
  }
  // at which point we want to set the perimeter with a new set of points..
  resetPerimeter(newPerimeter);   // this both sets the mask and does the drawing.
  borderSegments.resize(0);
  emit maskChanged();
}

// returns false if full perimeter, or segment is bad. If segment is bad.. will get incomplete
// perimeter. Not so good.
bool MaskMaker::addPoints(vector<QPoint>& points, vector<QPoint>& segment, set<int>& point_set)
{
  // check that the end is ok..
  if(!(maskValue( segment.back() ) & PERIMETER)){
    cerr << "MaskMaker::addPoints last segment point is not at an end position" << endl;
    return(false);
  }
  for(unsigned int i=0; i < segment.size(); ++i){
    int off = segment[i].y() * width + segment[i].x();
    if(point_set.count(off))
      return(false);
    point_set.insert(off);
    points.push_back(segment[i]);
    paintPoint(segment[i], 0, 0, 255, 255);
  }
  return(true);
}

bool MaskMaker::segmentFrom(QPoint& p, vector<QPoint>& points)
{
  for(unsigned int i=0; i < borderSegments.size(); ++i){
    if(borderSegments[i].size() && borderSegments[i][0] == p){
      points = borderSegments[i];
      return(true);
    }
  }
  return(false);
}

bool MaskMaker::perimeterIndexAt(QPoint& p, unsigned int& index)
{
  for(unsigned int i=0; i < perimeter.size(); ++i){
    if( p == perimeter[i]){
      index = i;
      return(true);
    }
  }
  return(false);
}

// At the moment the resetPerimeter function does both drawing and resetting of the mask
// I can do the drawing with a call to drawFromMask() instead. It would be marginally
// slower, but, might be better code
void MaskMaker::resetPerimeter(vector<QPoint>& points)
{
  int msize = width * height;
  perimeter = points;
  memset((void*)maskPicture, 0, 4 * sizeof(unsigned char) * msize);
  memset((void*)mask, 0, sizeof(unsigned char) * msize);
  for(unsigned int i=0; i < points.size(); ++i){
    int off = points[i].y() * width + points[i].x();
    if(off >= msize){
      cerr << "Bugger, resetPerimeter offset is too large " << endl;
      continue;
    }
    mask[off] = PERIMETER;
    off *= 4;
    maskPicture[ off ] = red;
    maskPicture[ off + 1] = green;
    maskPicture[ off + 2] = blue;
    maskPicture[ off + 3] = alpha;
  }
  //  memcpy((void*)maskPictureBuffer, (void*)maskPicture, sizeof(unsigned char) * 4 * msize);
}

void MaskMaker::drawFromMask()
{
  memset((void*)maskPicture, 0, 4 * sizeof(unsigned char) * width * height);
  for(unsigned int i=0; i < (width * height); ++i){
    if(!mask[i])
      continue;
    unsigned char r, g, b, a;
    r = red; g = green; b = blue; a = alpha;
    if(mask[i] & SEGMENT)
      b = 0;
    if(mask[i] & BEGIN)
      g = 0;
    if(mask[i] & END)
      r = 0;
    setRGBA(maskPicture + (4 * i), r, g, b, a);
  }
}

void MaskMaker::removeDuplicates(vector<QPoint>& points)
{
  vector<QPoint> newPoints;
  newPoints.reserve(points.size());
  set<int> pos;
  for(unsigned int i=0; i < points.size(); ++i){
    int off = points[i].y() * width + points[i].x();
    if(!pos.count( off )){
      newPoints.push_back(points[i]);
      pos.insert(off);
    }
  }
  points = newPoints;
}

void MaskMaker::paintPoint(QPoint& p, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  int o = 4 * (p.y() * width + p.x());
  setRGBA(maskPicture + o, r, g, b, a);
  emit maskChanged();
}

void MaskMaker::setMask()
{
  setMask(perimeter, PERIMETER, false);
  for(unsigned int i=0; i < borderSegments.size(); ++i)
    setMask(borderSegments[i], SEGMENT, true);
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
