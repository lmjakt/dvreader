#ifndef CELLOUTLINES_H
#define CELLOUTLINES_H

#include <vector>
#include "../spotFinder/perimeter.h"

struct CellOutlines {
  CellOutlines(std::vector<Perimeter> n, std::vector<Perimeter> c, unsigned short* mask){
    nuclei = n;
    cells = c;
    cellMask = mask;
  }
  CellOutlines(){
    cellMask = 0;
  }
  ~CellOutlines(){
    delete cellMask;
  }
  
  std::vector<Perimeter> nuclei;
  std::vector<Perimeter> cells;
  unsigned short* cellMask;
};

#endif
