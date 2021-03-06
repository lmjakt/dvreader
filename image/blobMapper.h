#ifndef BLOBMAPPER_H
#define BLOBMAPPER_H

#include "volumeMask.h"
#include "imageData.h"
#include "volumeMap.h"
#include "blob.h"
#include "superBlob.h"
#include "background.h"
#include "../dataStructs.h"
#include <vector>
#include <map>
#include <set>
#include <string>

class BlobModel;

struct BlobMapperInfo
{
  fluorInfo fluor;
  float minEdge;
  BlobMapperInfo(){
    minEdge = 0;
  }
  friend bool operator ==(const BlobMapperInfo& a, const BlobMapperInfo& b){
    return(a.fluor == b.fluor && a.minEdge == b.minEdge);
  }
};

class BlobMapper
{
 public:
  enum Param {
      VOLUME, SUM, MEAN, MAX, MIN, EXTENT, SURFACE, BACKGROUND, ASUM
  };

  BlobMapper(ImageData* ia, int xw, int yw, int zw, float pcnt);
  ~BlobMapper();

  void mapBlobs(float minEdge, unsigned int wi, int window, fluorInfo& finfo);
  bool exportBlobs(std::string fname);
  void setMapId(unsigned int mid);
  unsigned int mapId();
  float getParameter(blob* b, Param p);
  std::set<blob*>& gBlobs();
  void eatNeighbors();
  void setBackgroundParameters(int xw, int yw, int zw, float pcnt);
  
  std::vector<SuperBlob*> overlapBlobs(std::vector<BlobMapper*> mappers);
  blob* overlappingBlob(blob* b, BlobMapper* mapper);
  // peakWithinPeak is a recipocral function that looks for the presence of the peak within each other.
  blob* peaksWithinBlob(blob* b, BlobMapper* mapper);
  
  // models of the blobs in some reasonable format
  // two_dim_model, float is of the dimension (1 + (2 * z_radius)) * (1 + s_range)
  // s_range stands for surface range. It may be more reasonable to change the data.
  // or even to change it to something more useful. Plot using openGl or something?
  // or make dedicated plotter ? 
  float* two_dim_model(int& s_range, int& z_radius);  

  float minimum(){
    return(minimumEdge);
  }
  void dimensions(int& w, int& h, int& d){
    w = width; h = height; d = depth;
  }
  float g_value(off_set p);  // does a bounds checking..
  unsigned int blobNo(){
    return(blobs.size());
  }
  BlobMapperInfo info(){
    return(bmInfo);
  }
  unsigned int wave_index(){
    return(waveIndex);
  }
  std::vector<float> x_background(int y, int z, unsigned int ip);   // x background for pos y,z interpolated by ip
  
 private:
  struct NeighborInfo {
    std::vector<float> values;
    float sum;
    float peakValue;
    NeighborInfo(){
      sum = 0; peakValue = 0;
    }
    void addPoint(float v){
      values.push_back(v);
      sum += v;
      peakValue = peakValue < v ? v : peakValue;
    }
  };
  
  BlobMapperInfo bmInfo;
  //    std::map<off_set, blob*> blobMap;
  VolumeMap<blob*>* blobMap;
  Background* background;
  unsigned int map_id;
  std::set<blob*> blobs;
  std::set<blob*> uninterpol_blobs;  // only make if we use interpolation.. 
  bool uninterpol_up_to_date;        // are they up do date...
  int width, height, depth;
  unsigned int waveIndex;
  float minimumEdge;
  ImageData* image;
  //    VolumeMask* vMask; // I may not need it here.. 
  
  // and some internal functions to facilitate the mapping procedure.d
  blob* initBlob(blob* b, int x, int y, int z, int w);
  void extendBlob(int x, int y, int z, blob* b, int w, std::set<off_set>& currentPoints); // make this recursive maybe.
  bool isSurface(int x, int y, int z, blob* b, bool tight=false);
  void mergeBlobs(blob* newBlob, blob* oldBlob);        // new blob is taken by old blob, and references to old blob removed
  void addPointsToBlob(blob* tempBlob, blob* permBlob); // points from tempBlob are added to permBlob, tempBlob is cleared();
  void eatContainedBlobs();
  void eatContainedBlobs(blob* b);
  void eatContents(blob* b, VolumeMask* vm, int x, int y, int z);
  void eatNeighbors(blob* b);
  std::vector<off_set> findNeighbors(blob* b, off_set p);
  
  void finaliseBlobs(bool fake=false);
  void finaliseBlob(blob* b);
  void uninterpolateBlobs();
  void uninterpolate(blob* a, blob* b);
  void deleteBlobs(std::set<blob*> b);
  
  // blob modelling functions.
  void setModel(BlobModel* model, blob* b);

  // No error checking. 
  float value(int x, int y, int z){
    float v=0;
    image->point(v, x, y, z, waveIndex);
    return(v);
  }
  float value(off_set p){
    int x, y, z;
    toVol(p, x, y, z);
    return(value(x, y, z));
  }
  off_set linear(int x, int y, int z){
    return((off_set)z * ((off_set)width * (off_set)height) + 
	   ((off_set)y * (off_set)width) + (off_set)x);
  }
  void toVol(off_set p, int& x, int& y, int& z){
    z = p / (width * height);
    y = (p % (width * height)) / width;
    x = p % width;
  }
  // no bounds checking. Do elsewhere.
  bool sameBlob(int x, int y, int z, blob* b){
    return(b == blobMap->value(x, y, z));
  }
  bool differentBlob(int x, int y, int z, blob* b){
    blob* nblob = blobMap->value(x, y, z);
    if(!nblob)
      return(false);
    return(nblob != b);
  }
  int min(int v1, int v2){
    return( (v1 < v2 ? v1 : v2) );
  }
  int max(int v1, int v2){
    return( (v1 > v2 ? v1 : v2) );
  }
};

#endif
