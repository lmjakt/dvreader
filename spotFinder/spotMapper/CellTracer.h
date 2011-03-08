#ifndef CELLTRACER_H
#define CELLTRACER_H

#include "../../abstract/space.h"
#include "../perimeter.h"
#include <vector>



class CellTracer
{
 public:
  CellTracer(Space* pointSpace);
  ~CellTracer();
  
  unsigned char* makeCellMask(Perimeter& nucleus, std::vector<Point*>& points, int max_d,
			      int& mx, int& my, int& mw, int& mh);

  static const unsigned char nuc = 1;
  static const unsigned char node = 2;
  static const unsigned char edge = 4;
  static const unsigned char outside = 8;
  static const unsigned char border = 16;

 private:
  void initMask(Perimeter& nucleus, std::vector<Point*>& points);
  void drawDaughterConnections(std::vector<Point*>& points);
  void drawDaughterConnections(Point* point);
  void drawOrphanConnections(Point* point);
  std::vector<Point*> neighbors(Point* point);  // use for daughterless points

  void drawLine(int gx1, int gy1, int gx2, int gy2);  // global coordinates
  void floodExterior(int x, int y);

  Space* space;
  unsigned char* mask;
  int globalWidth;
  int maskWidth, maskHeight;
  int mask_x, mask_y;
  int maxD;  // max distance between points.
  
};

#endif
