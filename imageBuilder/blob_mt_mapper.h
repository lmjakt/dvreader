#ifndef BLOB_MT_MAPPER_H
#define BLOB_MT_MAPPER_H

#include <QThread>
#include <vector>
#include <string>
#include "../image/blob.h"
#include <map>

class ImStack;
class Blob;
class QSemaphore;
class BlobModel;
// Does essentially the same thing as ../image/blobMapper but uses and ImStack
// for data access.
// Rewritten completely for simplicity and to try and enable multithreading
// capability, as well as to split the problem into less memory intensive areas.

// Also assumes that ImStack is not particularly large and that we can use complete memory maps for
// simplicity.

class Blob_mt_mapper : public QThread
{
  Q_OBJECT  // enable signals and slots
    
    public:
  Blob_mt_mapper(ImStack* imStack, unsigned int mapper_id, bool free_memory=false);
  ~Blob_mt_mapper();
  void setBgPar(uint xm, uint ym, float q);
  void setBlobModel(BlobModel* b_model);  // set the blobModel to b_model
  void mapBlobs(unsigned int wi, float minimumEdge, float minimumPeak, QSemaphore* sem=0); // this will call run()
  std::string description();
  void position(int& x, int& y, int& z);
  void dims(unsigned int& w, unsigned int& h, unsigned int& d);
  std::vector<blob*> rblobs();  // shouldn't really be rblobs, .. 
  // we need some way of returning the blobs in a reasonable structure
  // For now use the old ../image/blob.h definition. But note that it is probably too memory intensive.
 signals:
  void error(const char*);
  
 private:
  void run();
  void subtract_background();
  blob* initBlob(blob* b, uint x, uint y, uint z, float v);
  void extendBlob(int x, int y, int z, blob* b);
  void mergeBlobs(blob* newBlob, blob* oldBlob);        // new blob is incoporated into old blob, references to old blob removed
  bool isSurface(int x, int y, int z, blob* b, bool tight=false);
  void finaliseBlobs();
  void finaliseBlob(blob* b);
  void incrementBlobModel(blob* b);
  void deleteBlobs();
  void freeMemory();
  void determineBlobCenter(blob* b, float& fx, float& fy, float& fz);

  off_set linear(int x, int y, int z){
    return( z * stack_width * stack_height + y * stack_width + x);
  }
  void toVol(off_set p, int& x, int& y, int& z){
    z = p / (stack_width * stack_height);
    y = (p % (stack_width * stack_height)) / stack_width;
    x = p % stack_width;
  }

  struct stack_info {
    int x, y, z;
    uint w, h, d;
  };

  ImStack* stack;
  stack_info pos;
  QSemaphore* qsem;
  bool destroy_memory;  // if true destroys imageStack, blobMap, and mask after mapping blobs. 
  unsigned int stack_channel;
  std::vector<blob*> temp_blobs;
  static const unsigned int Left = 1;
  static const unsigned int Right = 2;
  static const unsigned int Top = 4;
  static const unsigned int Bottom = 8;  
  std::multimap<unsigned int, blob*> blobs;     // blob defined as being, inside, on edges or corners.. 
  BlobModel* blobModel;                  // an optional model to which points can be added. 
  int stack_width, stack_height, stack_depth;
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
