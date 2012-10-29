#include "Drawer.h"
#include <stdlib.h>
#include <string.h>
#include "blob_set.h"
#include "stack_info.h"
#include "../image/blob.h"

using namespace std;

Drawer::Drawer()
{
  globalWidth = globalHeight = xoff = yoff = 0;
  picture = 0;
  setBrushColor(QColor(255, 255, 255, 100));
  setPenColor(QColor(255, 255, 255, 255));
}

Drawer::Drawer(unsigned char* pic, int x, int y, int w, int h)
{
  globalWidth = globalHeight = xoff = yoff = 0;
  picture = 0;
  setPic(pic, x, y, w, h);
  setBrushColor(QColor(255, 255, 255, 100));
  setPenColor(QColor(255, 255, 255, 255));
}

Drawer::~Drawer()
{
}

void Drawer::setPic(unsigned char* pic, int x, int y, int w, int h)
{
  if(w <= 0 || h <= 0)
    return;
  globalWidth = w;
  globalHeight = h;
  xoff = x;
  yoff = y;
  picture = pic;
}

void Drawer::setPenColor(QColor c)
{
  setColor(c, &pColor);
}

void Drawer::setBrushColor(QColor c)
{
  setColor(c, &bColor);
}

void Drawer::setBackground(QColor bg)
{
  if(!picture)
    return;
  unsigned char color[4];
  color[0] = bg.red();
  color[1] = bg.green();
  color[2] = bg.blue();
  color[2] = bg.alpha();
  // if no color we can use memset, otherwise our own loop.
  if(!(*(int*)color)){     // i.e. all values 0
    memset((void*)picture, 0, sizeof(unsigned char) * 4 * globalWidth * globalHeight);
    return;
  }
  unsigned int* dest = (unsigned int*)picture;
  unsigned int* dest_end = dest + (globalWidth * globalHeight);
  unsigned int col = *(int*)color;
  while(dest < dest_end){
    *dest = col;
    ++dest;
  }
}

void Drawer::drawLines(vector<QPoint> points, QColor penColor, bool close)
{
  setColor(penColor, &pColor);
  drawLines(points, close, false);
}

// Fill not implemented in this version.
// option for solid also not set yet.. 
void Drawer::drawLines(vector<QPoint> points, bool close, bool fill)
{
  for(unsigned int i=1; i < points.size(); ++i)
    drawLine(points[i-1], points[i], false);
  if(close)
    drawLine(points.back(), points[0], false);
}

void Drawer::drawCell(Cell2 cell, set<unsigned int> blob_ids, bool use_corrected)
{
  drawLines(cell.cellPerimeter().qpoints(), true, false);
  drawLines(cell.nucleusPerimeter().qpoints(), true, false);
  vector<blob_set*> blobs = cell.blobs(blob_ids, use_corrected);
  for(unsigned int i=0; i < blobs.size(); ++i)
    draw_blob_set( blobs[i] );

  blobs = cell.burst_blobs(blob_ids, use_corrected);
  for(unsigned int i=0; i < blobs.size(); ++i)
    draw_blob_set( blobs[i] );

}

void Drawer::draw_blob_set(blob_set* bs)
{
  stack_info pos = bs->position();
  vector<blob*> blobs = bs->b();
  for(unsigned int i=0; i < blobs.size(); ++i){
    unsigned int peak_slice = blobs[i]->peakPos / (pos.w * pos.h);
    for(unsigned int j=0; j < blobs[i]->points.size(); ++j){
      if(blobs[i]->surface[j] && blobs[i]->points[j] / (pos.w * pos.h) == peak_slice)
	drawPoint( pos.x + (blobs[i]->points[j] % pos.w), 
		   pos.y + (blobs[i]->points[j] % (pos.w * pos.h)) / pos.w);
    }
  }
}

void Drawer::drawPoint(int x, int y)
{
  x -= xoff;
  y -= yoff;
  if(x < 0 || x >= globalWidth)
    return;
  if(y < 0 || y >= globalWidth)
    return;
  unsigned int* ipic = (unsigned int*)picture;
  ipic[y * globalWidth + x] = pColor;
}

void Drawer::pos_dims(int& x, int& y, int& w, int& h)
{
  w = globalWidth;
  h = globalHeight;
  x = xoff;
  y = yoff;
}

void Drawer::setColor(QColor& qc, unsigned int* col)
{
  unsigned char* cptr = (unsigned char*)col;
  cptr[0] = qc.red();
  cptr[1] = qc.green();
  cptr[2] = qc.blue();
  cptr[3] = qc.alpha();
}

void Drawer::drawLine(QPoint& p1, QPoint& p2, bool solid)
{
  int dx = (p2.x() - p1.x());
  int dy = (p2.y() - p1.y());
  int nstep = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
  
  int* ipic = (int*)picture;
  // special case if p1 == p2, just draw one point and return the point.
  if(!nstep){
    // do stupid stuff
    ////////// bug, bug.. that should be - xoff and - yoff.. 
    /////////  ARGH>>> <<<<
    int x = p1.x() - xoff;
    int y = p1.y() - yoff;
    if(x < 0 || y < 0 || x >= globalWidth || y > globalHeight)
      return;
    ipic[y * globalWidth + x] = pColor;
    return;
  }
  int lx = p1.x();
  int ly = p1.y(); // last_x & y used in order to prevent holes diagonals  
  for(int i=0; i <= nstep; ++i){
    int x = (p1.x() + (i * dx) / nstep) - xoff;
    int y = (p1.y() + (i * dy) / nstep) - yoff;
    if(x < 0 || x >= globalWidth) continue;
    if(y >= globalHeight || y < 0) continue;
    if(solid && (x - lx) && (y - ly) ){
      ipic[y * globalWidth + lx] = pColor;
    }
    ipic[y * globalWidth + x] = pColor;
    lx = x;
    ly = y;
  }
}
