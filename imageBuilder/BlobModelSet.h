#ifndef BLOBMODELSET_H
#define BLOBMODELSET_H

#include "blob_set.h"
#include <vector>
#include <map>

class BlobModel;
class Blob_mt_mapper;

struct blob;

// Contains a set of models for a given blob_set type
// Just a container to enhance readability of code ?


class BlobModelSet {
 public:
  BlobModelSet(unsigned int set_id, int xy_radius, int z_radius);
  ~BlobModelSet();
  void trainModels(std::vector<blob_set>& bsets);
  void assessBlobs(std::vector<blob_set>& bsets);
  BlobModel* model(unsigned int mapper_id);
  
 private:
  std::map<unsigned int, BlobModel*> models;
  unsigned int super_id;
  int xyRadius, zRadius;

  void trainModel(BlobModel* model, std::vector<blob*>& blobs, Blob_mt_mapper* mapper);
  std::map<Blob_mt_mapper*, std::vector<blob*> > arrangeBlobs(std::vector<blob_set>& bsets);
  bool checkIds(blob_set& bs);
  void deleteModels();
};

#endif
