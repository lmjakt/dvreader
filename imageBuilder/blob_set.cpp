#include "blob_set.h"
#include "../image/blob.h"
#include "blob_mt_mapper.h"

using namespace std;

void blob_set::push(blob* b, unsigned int m_id, Blob_mt_mapper* mapper)
{
  blobs.push_back(b);
  mapper_ids.push_back(m_id);
  mappers.push_back(mapper);
  set_id |= m_id;
  corrected_id |= m_id;
}

vector<blob*> blob_set::b()
{
  return(blobs);
}

vector<unsigned int> blob_set::ids()
{
  return(mapper_ids);
}

vector<Blob_mt_mapper*> blob_set::bms()
{
  return(mappers);
}

unsigned int blob_set::size(){
  return(blobs.size());
}

blob* blob_set::b(unsigned int i){
  if(i < blobs.size())
    return(blobs[i]);
  return(0);
}

blob* blob_set::blob_with_id(unsigned int id){
  for(unsigned int i=0; i < mapper_ids.size(); ++i){
    if(mapper_ids[i] == id)
      return(blobs[i]);
  }
  return(0);
}

unsigned int blob_set::bid(unsigned int i){
  if(i < blobs.size())
    return(mapper_ids[i]);
  return(0);
}

Blob_mt_mapper* blob_set::bm(unsigned int i){
  if(i < blobs.size())
    return(mappers[i]);
  return(0);
}

stack_info blob_set::position(){
  if(blobs.size())
    return(mappers[0]->position());
  return(stack_info());
}

unsigned int blob_set::id()
{
  return(set_id);
}

void blob_set::setCorrectedId(unsigned int new_id){
  if(new_id <= set_id)
    corrected_id = new_id;
}

unsigned int blob_set::correctedId(){
  return(corrected_id);
}
