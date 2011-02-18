#ifndef PERIMETERSPLITTER_H
#define PERIMETERSPLITTER_H

#include <vector>
#include <map>
#include "perimeter.h"

class PerimeterSplitter {
 public:
  PerimeterSplitter();
  ~PerimeterSplitter();

  void splitPerimeter(Perimeter& per, std::vector<std::vector<int> >& splitLines);
  unsigned int* perimeterMask(int& mw, int& mh);
  std::map<unsigned int, std::vector<int> > newPerimeters();
  std::vector<std::vector<int> > newPerimetersV();

 private:
  unsigned int* mask;
  int globalWidth;
  int mask_x, mask_y;
  int maskWidth, maskHeight;
  std::vector<unsigned int> areaIds;

  void initMask(Perimeter& per);
  void addLineToMask(std::vector<int>& points);
  void finaliseMask();
  void flood_or_mask(int x, int y, unsigned int forbidden, unsigned int flood_value, bool set_value);  // uses local parameters!!!
  std::vector<int> tracePerimeter(unsigned int p_id);
};



#endif
