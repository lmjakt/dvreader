#ifndef CELL2_H
#define CELL2_H

#include <set>
#include <vector>
#include "../spotFinder/perimeter.h"
#include "blob_set.h"
#include <fstream>

class Cell2 {
 public:
  Cell2();
  Cell2(Perimeter& c, Perimeter& n);
  ~Cell2();

  bool contains(blob_set* bs);
  bool addBlob(blob_set* bs);
  void clearBlobs();
  std::set<blob_set*> blobs();  // does not have ownership!
  void setCellPerimeter(Perimeter& cp);
  Perimeter cellPerimeter();
  Perimeter nucleusPerimeter();
  void writeTextSummary(std::ofstream& out);
  bool writePerimeters(std::ofstream& out);
  bool readPerimeters(std::ifstream& in);
  std::vector<blob_set*> blobs(std::set<unsigned int> blob_ids, bool use_corrected);

 private:
  Perimeter cell;
  Perimeter nucleus;
  std::set<blob_set*> blob_sets;
};

#endif
