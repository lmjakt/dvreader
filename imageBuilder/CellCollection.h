#ifndef CELLCOLLECTION_H
#define CELLCOLLECTION_H

#include "Cell2.h"
#include "../spotFinder/perimeter.h"
#include "blob_set.h"
#include <vector>
#include <set>

class CellCollection {
 public:
  CellCollection();
  ~CellCollection();

  void addCell(Perimeter& cellP, Perimeter& nucP);
  void addBlobs(std::vector<blob_set>& bs);
  void addBlob(blob_set& bs);
  unsigned int cellNumber();
  int currentCell();
  bool cell(unsigned int i, Cell2& cell);
  std::vector<blob_set> cellBlobs(unsigned int i);

  void setCurrentCell(int ccell);
  bool modifyCellPerimeter(unsigned int i, Perimeter& cellP);
  Perimeter cellPerimeter(unsigned int i);
  Perimeter nucleusPerimeter(unsigned int i);
  void reassignBlobs();

 private:
  void addBlob(blob_set* bsptr);
  std::vector<Cell2> cells;
  std::set<blob_set*> blobs;
  std::set<blob_set*> unallocated_blobs;
  std::set<blob_set*> conflicting_blobs;
  int current_cell; // defaults to -1
};

#endif
