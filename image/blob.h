#ifndef BLOB_H
#define BLOB_H

#include <vector>
#include <QString>
#include "../imageBuilder/stack_info.h"

class BlobMapper;
class Blob_mt_mapper;
class c_array;

typedef unsigned int off_set;
typedef unsigned int uint;

struct blob {
  blob(){
    min = max = 0;
    peakPos = 0;
    r = g = b = 200;
    active = true;
    b_class = super_class = 0;
    class_lr = 0;
    aux1=0;
  }
  ~blob(){
  }
  void size(uint& s);
  void serialise(c_array* buf);
  std::vector<off_set> points;
  std::vector<bool> surface; 
  float min, max, sum;
  float aux1;  // some auxiliary value that can be set by any owner
  int max_x, min_x, max_y, min_y, max_z, min_z;
  off_set peakPos;
  //    unsigned long peakPos;
  int r, g, b;
  bool active;
  int b_class;
  float class_lr;
  int super_class;
  Blob_mt_mapper* mapper;
};

// The below is just a way of keeping a reference to a blob along
// with some sort of an identifier. (How that is used is up to the
// user, but I'm planning to use it with up to 31 mapper ids using
// binary OR's to allow easy classification of multiple membership.
struct id_blob {
    id_blob(){
	mapper_id = 0;
	b = 0;
	mapper = 0;
    }
    id_blob(unsigned int id, blob* bl, BlobMapper* m){
	b = bl;
	mapper_id = id;
	mapper = m;
    }
    unsigned int mapper_id;
    blob* b;
    BlobMapper* mapper;
};

// contains the 
struct blob_space {
  float* values;
  bool* membership;
  stack_info pos;

  blob_space(){
    values = 0;
    membership = 0;
  }
  blob_space(float* v, bool* bm, stack_info s_pos){
    values = v;
    membership = bm;
    pos = s_pos;
  }
  ~blob_space(){
    delete []values;
    delete []membership;
  }
};

// stupid ugly hack.. 
float getBlobParameter(blob* b, QString parname);


#endif

