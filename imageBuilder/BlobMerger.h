#ifndef BLOBMERGER_H
#define BLOBMERGER_H

#include <vector>
#include <set>
#include "blob_set.h"
#include "../datastructs/channelOffset.h"

class Blob_mt_mapper;
class blob_set;

struct blob;
struct stack_info;

class BlobMerger {
 public:
  BlobMerger();
  ~BlobMerger();
  // the use of ChannelOffset here is bad, and is something that I should remove
  // after fixing the segmentation fault resulting from using this with the
  // global channel offset settings.
  std::vector<blob_set> mergeBlobs(std::vector<Blob_mt_mapper*> bl_mappers, std::vector<ChannelOffset> ch_offsets, unsigned int r);
  
 private:
  std::vector<Blob_mt_mapper*> mappers;
  std::vector<ChannelOffset> channelOffsets;
  std::vector<blob**> maps;  // a width * height * depth array for each mapper
  std::vector<int> offsets; // the offsets of neighbor positions..
  stack_info pos;
  int radius;

  void initOffsets();
  std::vector<int> adjustOffsets(unsigned int map_id);
  void initMaps();
  void initMap(Blob_mt_mapper* mapper, blob** map);
  std::vector<blob*> getNeighborBlobs(unsigned int map_id, int p);
  bool isComplete(std::vector<blob*> bv);
  std::vector<blob_set> merge();

};

#endif
