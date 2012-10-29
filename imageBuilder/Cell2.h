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
  bool nucleus_contains(blob_set* bs);
  bool addBlob(blob_set* bs);
  void addBurstBlob(blob_set* bs); // does not check location, use nucleus_contains
  void clearBlobs();
  void clearBurstBlobs();
  std::set<blob_set*> blobs();  // does not have ownership!
  std::set<blob_set*> burst_blobs();
  void setCellPerimeter(Perimeter& cp);
  Perimeter cellPerimeter();
  Perimeter nucleusPerimeter();
  void writeTextSummary(std::ofstream& out);
  bool writePerimeters(std::ofstream& out);
  bool readPerimeters(std::ifstream& in);
  std::vector<blob_set*> blobs(std::set<unsigned int> blob_ids, bool use_corrected);
  std::vector<blob_set*> burst_blobs(std::set<unsigned int> blob_ids, bool use_corrected);

 private:
  Perimeter cell;
  Perimeter nucleus;
  std::set<blob_set*> blob_sets;
  std::set<blob_set*> nuclear_bursts;  // should represent sites of active transcription
};

#endif
