#ifndef BLOB_MT_MAPPER_H
#define BLOB_MT_MAPPER_H

#include <QThread>
#include <vector>
#include <string>
#include "../datastructs/channelOffset.h"
#include "../image/blob.h"
#include "blob_set.h"
#include "stack_info.h"
#include <math.h>
#include <map>

class ImStack;
class Blob;
class QSemaphore;
class BlobModel;
class FileSet;
class BlobMerger;
class c_array;
// Does essentially the same thing as ../image/blobMapper but uses and ImStack
// for data access.
// Rewritten completely for simplicity and to try and enable multithreading
// capability, as well as to split the problem into less memory intensive areas.

// Also assumes that ImStack is not particularly large and that we can use complete memory maps for
// simplicity.

class Blob_mt_mapper : public QThread
{
  Q_OBJECT  // enable signals and slots
    
  friend class BlobMerger;
 public:
  Blob_mt_mapper(stack_info s_info, FileSet* fset, unsigned int mapper_id, bool free_memory=false);
  Blob_mt_mapper(ImStack* imStack, unsigned int mapper_id, bool free_memory=false);
  ~Blob_mt_mapper();
  void setBgPar(uint xm, uint ym, float q);
  void setBlobModel(BlobModel* b_model);  // set the blobModel to b_model
  void addToBlobModel(BlobModel* bmodel, std::vector<blob*>& blbs);
  void mapBlobs(unsigned int wi, float minimumEdge, float minimumPeak, QSemaphore* sem=0); // this will call run()
  std::string description();
  void position(int& x, int& y, int& z);
  void dims(unsigned int& w, unsigned int& h, unsigned int& d);
  blob_space* blobSpace(blob* b);
  std::vector<blob*> rblobs();  // shouldn't really be rblobs, ..
  void reset_model_correlations();
  std::vector<float> blob_model_correlations();
  std::vector<float> blob_model_correlations(BlobModel* bmodel, std::vector<blob*>& local_blobs);
  std::vector<blob_set> blob_sets(std::vector<Blob_mt_mapper*> mappers, std::vector<ChannelOffset> offsets,
				  unsigned int radius);
  stack_info position(){ return pos; }
  unsigned int mapId(){ return map_id; }
  void setImageStack(bool sub_background);
  void freeImageStack();

  void serialise(c_array* buf);
  // we need some way of returning the blobs in a reasonable structure
  // For now use the old ../image/blob.h definition. But note that it is probably too memory intensive.

  // global peak positions for member blobs.
  bool blob_peak_pos(blob* b, int& x, int& y, int& z);
  unsigned int channel(); // only set if mapBlobs has been called.

 signals:
  void error(const char*);
  
 private:
  void setBlobMap(); // sets the blobMap and the mask (blob) using any blobs defined.. 
  void run();
  void subtract_background();
  blob* initBlob(blob* b, uint x, uint y, uint z, float v);
  void extendBlob(int x, int y, int z, blob* b);
  void mergeBlobs(blob* newBlob, blob* oldBlob);        // new blob is incoporated into old blob, references to old blob removed
  bool isSurface(int x, int y, int z, blob* b, bool tight=false);
  void finaliseBlobs();
  void finaliseBlob(blob* b);
  void incrementBlobModel(blob* b);
  void incrementBlobModel(blob* b, BlobModel* model);
  void deleteBlobs();
  //  void overlappingBlobs(blob_set& bset);
  void freeMemory();
  void determineBlobCenter(blob* b, float& fx, float& fy, float& fz);
  void z_normalise(float* v, unsigned int length);
  blob* blob_at(off_set pos);  // creates a blobMap if not present.. 

  off_set linear(int x, int y, int z){
    return( z * pos.w * pos.h + y * pos.w + x);
  }
  void toVol(off_set p, int& x, int& y, int& z){
    z = p / (pos.w * pos.h);
    y = (p % (pos.w * pos.h)) / pos.w;
    x = p % pos.w;
  }
  float e_dist(float x1, float y1, float x2, float y2){
    return(sqrt( (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) ));
  }
  bool isPowerOfTwo(unsigned int i){
    return( (i != 0) && !(i & (i - 1)) );
  }

  std::vector<Blob_mt_mapper*> unmix_id(unsigned int i, std::map<unsigned int, Blob_mt_mapper*>& key){
    std::vector<Blob_mt_mapper*> mappers;
    for(std::map<unsigned int, Blob_mt_mapper*>::iterator it=key.begin(); it != key.end(); ++it){
      if( i & (*it).first)
	mappers.push_back((*it).second);
    }
    return(mappers);
  }

  
  ImStack* stack;
  FileSet* fileSet;
  stack_info pos;
  QSemaphore* qsem;
  bool destroy_memory;  // if true destroys imageStack, blobMap, and mask after mapping blobs. 
  unsigned int stack_channel;
  unsigned int wave_index;  // the global channel
  std::vector<blob*> temp_blobs;
  static const unsigned int Left = 1;
  static const unsigned int Right = 2;
  static const unsigned int Top = 4;
  static const unsigned int Bottom = 8;  
  std::multimap<unsigned int, blob*> blobs;     // blob defined as being, inside, on edges or corners.. 
  BlobModel* blobModel;                  // an optional model to which points can be added. 
  //  int stack_width, stack_height, stack_depth; // for inline functions to avoid pos.w ?
  blob** blobMap;   // size equal to the volume of the ImStack. // to save memory we could use an offset to blobs hmm.
  bool* mask;    // size equal to the volume of the ImStack.
  unsigned int map_id;  // should 1 2 4 8 16, etc.
  float minEdge;
  float minPeak;
  
  // subtract background ?
  uint bg_xm, bg_ym;
  float bg_q;
};


#endif
