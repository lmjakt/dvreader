#ifndef BLOB_H
#define BLOB_H

#include <vector>

typedef unsigned int off_set;
typedef unsigned int uint;

struct blob {
    blob(){
	min = max = 0;
	peakPos = 0;
    }
  ~blob(){
    for(uint i=0; i < blobs.size(); ++i)
  	    delete blobs[i];
  }

  void deleteChildren(){
    for(uint i=0; i < blobs.size(); ++i)
      delete blobs[i];
  }
    void flatten();
    void flatten(blob* parentBlob);
    void size(uint& s);
    void childNo(uint& c);

    std::vector<off_set> points;
    std::vector<float> values;
    std::vector<bool> surface; 
    std::vector<blob*> blobs;
    float min, max;
    int max_x, min_x, max_y, min_y, max_z, min_z;
    unsigned long peakPos;
};

#endif

