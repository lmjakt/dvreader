#ifndef DRAWER_H
#define DRAWER_H

#include <QPoint>
#include <QColor>
#include <vector>
#include <set>
#include "Cell2.h"

// Drawer draws things on an RGBA byte representation (char*) given to it.
// Drawer does not own anything it draws on..

class Drawer
{
 public:
  Drawer();
  Drawer(unsigned char* pic, int x, int y, int w, int h);
  ~Drawer();

  void setPic(unsigned char* pic, int x, int y, int w, int h);
  void setPenColor(QColor c);
  void setBrushColor(QColor c);
  void setBackground(QColor bg);
  void drawLines(std::vector<QPoint> points, QColor penColor, bool close);
  void drawLines(std::vector<QPoint> points, bool close, bool fill);
  void drawCell(Cell2 cell, std::set<unsigned int> blob_ids, bool use_corrected);
  void draw_blob_set(blob_set* bs);
  void drawPoint(int x, int y);

  void pos_dims(int& x, int& y, int& w, int& h);

 private:
  int globalWidth, globalHeight;
  int xoff, yoff;  // the position of the area
  unsigned char* picture;
  unsigned int pColor;
  unsigned int bColor;
  
  void setColor(QColor& qc, unsigned int* col);
  void drawLine(QPoint& p1, QPoint& p2, bool solid);  // solid = whether or not diagonal gaps are allowed
};

#endif
