#ifndef CELL2_H
#define CELL2_H

#include <set>
#include "../spotFinder/perimeter.h"
#include "blob_set.h"

class Cell2 {
 public:
  Cell2();
  Cell2(Perimeter& c, Perimeter& n);
  ~Cell2();

  bool contains(blob_set* bs);
  bool addBlob(blob_set* bs);
  std::set<blob_set*> blobs();  // does not have ownership!
  void setCellPerimeter(Perimeter& cp);
  Perimeter cellPerimeter();
  Perimeter nucleusPerimeter();

 private:
  Perimeter cell;
  Perimeter nucleus;
  std::set<blob_set*> blob_sets;
};

#endif
