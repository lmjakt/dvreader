#ifndef BLOBCLASSIFIER_H
#define BLOBCLASSIFIER_H

#include <vector>
#include <set>
#include <map>
#include "nd_classifier.h"
#include "blobMapper.h"


class BlobClassifier 
{
 public:
  BlobClassifier();
  ~BlobClassifier();
  
  void setSuperBlobs(std::vector<SuperBlob*>& sblobs);
  void setParameters(std::set<BlobMapper::Param> params);

 private:
  std::map<uint, BlobMapper*> bmwMap;
  // For each blobMapper we have a map of blobs, where
  // the key is the class that blob belongs to.
  std::map<BlobMapper*, std::map<uint, std::vector<blob*> > > blobs;
  std::map<BlobMapper*, ND_Classifier*> classifiers;
  std::set<BlobMapper::Param> classParameters;
  
  void clearData();
  void trainClassifiers();   // no real training just sets the data.
  void trainClassifier(BlobMapper* mapper);
  void setTrainData(float* data, blob* b, BlobMapper* bm);
  
};

#endif
