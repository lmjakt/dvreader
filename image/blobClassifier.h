#ifndef BLOBCLASSIFIER_H
#define BLOBCLASSIFIER_H

#include <vector>
#include <set>
#include <map>
#include "nd_classifier.h"
#include "blobMapper.h"

struct BlobClassCounts
{
  BlobMapper* mapper;
  std::map<int, uint> counts;
  std::map<int, std::map<int, uint> > class_super_counts;
  BlobClassCounts(){
    mapper = 0;
  }
  BlobClassCounts(BlobMapper* m){
    mapper = m;
  }
};

class BlobClassifier 
{
 public:
  BlobClassifier();
  ~BlobClassifier();
  
  void setSuperBlobs(std::vector<SuperBlob*>& sblobs);
  void setParameters(std::set<BlobMapper::Param> params);
  void removeBlobMapper(BlobMapper* bm);
  std::map<BlobMapper*, std::map<uint, std::vector<blob*> > > gBlobs();
  std::vector<BlobClassCounts> classifyBlobs(std::vector<BlobMapper*> mappers);

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
  BlobClassCounts classifyBlobs(BlobMapper* mapper, float** classData, std::vector<int>& classes, bool normalise);
  float* extractBlobData(std::set<blob*> b, BlobMapper* bm);
  std::map<int, uint> countClasses(float* classData, std::set<blob*>& b, std::vector<int>& classes, bool setBlobs);
  std::map<int, std::map<int, uint> > count_ClassSuperCounts(std::set<blob*>& b);
  
};

#endif
