#ifndef BLOB_SET_H
#define BLOB_SET_H

#include <vector>
#include "stack_info.h"

struct blob;
class Blob_mt_mapper;

class blob_set {
  std::vector<blob*> blobs;
  std::vector<unsigned int> mapper_ids;
  std::vector<Blob_mt_mapper*> mappers;
  unsigned int set_id;
  unsigned int corrected_id;

 public :
  blob_set(){
    set_id = 0;
    corrected_id = 0;
  }
  
  void push(blob*, unsigned int m_id, Blob_mt_mapper* mapper);
  std::vector<blob*> b();
  std::vector<unsigned int> ids();
  std::vector<Blob_mt_mapper*> bms();
  
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

#endif
