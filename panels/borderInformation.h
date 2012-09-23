#ifndef BORDERINFORMATION_H
#define BORDERINFORMATION_H

#include <QPoint>
#include <map>
#include <set>
#include <string.h>

struct color_map;

enum POSITION {
  LEFT, TOP, RIGHT, BOTTOM
};

struct BorderArea {
  float** t_data;
  float** n_data;
  unsigned int* t_bleach_count;
  unsigned int* n_bleach_count;
  int* wave_lengths;
  int wave_no;
  int width, height;
  int x, y;
  BorderArea();
  BorderArea(float** td, float** nd, int* wave_l, int wave_n, int xp, int yp, int w, int h);
  BorderArea(float** td, float** nd, int* wave_l, int wave_n, uint* tbleach, uint* nbleach, int xp, int yp, int w, int h);
  ~BorderArea();
};


class BorderInfo {
 public:
  BorderInfo(float x, float y);
  ~BorderInfo();
  void setArea(BorderArea* ba, POSITION pos);
  // makes a new float*, of width h, and height h
  void setBias(float b);
  void setScale(float s);
  void setScaleAndBias(float s, float b);
  void setOffset(QPoint p);
  QPoint offset();
  float* paint_overlap(POSITION pos, int wl, int& w, int& h, color_map& t_color, color_map& n_color);
  float x(){ return(x_pos); }
  float y(){ return(y_pos); }
  std::set<int> channels();

 private:
  std::map<POSITION, BorderArea*> areas;
  QPoint offSet;
  float bias, scale;
  float x_pos, y_pos;
};
    
#endif
