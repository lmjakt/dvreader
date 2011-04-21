#ifndef BLOBMERGER_H
#define BLOBMERGER_H

#include <vector>
#include <set>
#include "blob_set.h"

class Blob_mt_mapper;
class blob_set;

struct blob;
struct stack_info;

class BlobMerger {
 public:
  BlobMerger();
  ~BlobMerger();
  std::vector<blob_set> mergeBlobs(std::vector<Blob_mt_mapper*> bl_mappers, unsigned int r);
  
 private:
  std::vector<Blob_mt_mapper*> mappers;
  std::vector<blob**> maps;  // a width * height * depth array for each mapper
  std::vector<int> offsets; // the offsets of neighbor positions..
  stack_info pos;
  int radius;

  void initOffsets();
  void initMaps();
  void initMap(Blob_mt_mapper* mapper, blob** map);
  std::vector<blob*> getNeighborBlobs(unsigned int map_id, int pos);
  bool isComplete(std::vector<blob*> bv);
  std::vector<blob_set> merge();

};

#endif
