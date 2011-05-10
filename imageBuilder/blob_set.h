#ifndef BLOB_SET_H
#define BLOB_SET_H

#include <vector>
#include "stack_info.h"

struct blob;
struct blob_space;

class Blob_mt_mapper;

class blob_set {
  friend struct blob_set_space;

  std::vector<blob*> blobs;
  std::vector<unsigned int> mapper_ids;
  std::vector<Blob_mt_mapper*> mappers;
  unsigned int set_id;
  unsigned int corrected_id;
  int offset_x, offset_y, offset_z;

 public :
  const static unsigned int GOOD = 0;
  const static unsigned int MULTI = 1;
  const static unsigned int INCOMPLETE = 2;
  const static unsigned int EXTENSIBLE = 3;
  unsigned int error;
  blob_set(){
    set_id = 0;
    corrected_id = 0;
    offset_x = offset_y = offset_z = 0;
    error = GOOD;
  }
  blob_set(int xo, int yo, int zo){
    offset_x = xo;
    offset_y = yo;
    offset_z = zo;
    set_id = 0;
    corrected_id = 0;
    error = GOOD;
  }
  ~blob_set(){}
  void adjust_pos(int& x, int& y, int& z){
    x += offset_x;
    y += offset_y;
    z += offset_z;
  }

  void push(blob*, unsigned int m_id, Blob_mt_mapper* mapper);
  void addError(unsigned int ef){
    error |= ef;
  }
  std::vector<blob*> b();
  std::vector<unsigned int> ids();
  std::vector<Blob_mt_mapper*> bms();
  
  void mg_pos(int& x, int& y, int& z);  // mean global position.
  unsigned int size();
  blob* b(unsigned int i);
  blob* blob_with_id(unsigned int id);
  unsigned int bid(unsigned int i);
  Blob_mt_mapper* bm(unsigned int i);
  stack_info position();
  unsigned int id();
  unsigned int correctedId();
  void setCorrectedId(unsigned int new_id);
};

class blob_set_space {
 public:
  blob_set_space();
  blob_set_space( blob_set bs );
  ~blob_set_space();

  unsigned int size();
  blob_space* space(unsigned int i);
  blob* b(unsigned int i);
  blob_set bs();   // but beware the blob* pointers won't match up.

 private:
  std::vector<blob_space*> spaces;
  std::vector<blob*> blobs;
  blob_set b_set;
};

#endif
