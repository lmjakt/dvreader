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

void blob_set::mg_pos(int& x, int& y, int& z)
{
  x = y = z = 0;
  if(!mappers.size())
    return;
  unsigned int w, h, d;
  mappers[0]->dims(w, h, d);
  for(unsigned int i=0; i < blobs.size(); ++i){
    x += (blobs[i]->peakPos % w);
    y += ( (blobs[i]->peakPos % (w * h)) / w );
    z += (blobs[i]->peakPos / (w * h));
  }
  x = (x / blobs.size()) + offset_x;
  y = (y / blobs.size()) + offset_y;
  z = (z / blobs.size()) + offset_z;
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

///////// constructors for additional struct blob_set_space
///////// only a small container

blob_set_space::blob_set_space()
{
}

blob_set_space::blob_set_space( blob_set bs )
{
  b_set = bs;
  for(unsigned int i=0; i < bs.blobs.size(); ++i){
    spaces.push_back( bs.mappers[i]->blobSpace( bs.blobs[i] ));
    blobs.push_back( new blob(*bs.blobs[i]) );  
    //blobs.push_back( bs.blobs[i] );

  }
}

blob_set_space::~blob_set_space()
{
  for(unsigned int i=0; i < blobs.size(); ++i){
    delete blobs[i];
    delete spaces[i];
  }
}

unsigned int blob_set_space::size()
{
  return(spaces.size());
}

blob_space* blob_set_space::space(unsigned int i)
{
  if(i < spaces.size())
    return(spaces[i]);
  return(0);
}

blob* blob_set_space::b(unsigned int i)
{
  if(i < blobs.size())
    return(blobs[i]);
  return(0);
}

blob_set blob_set_space::bs()
{
  return(b_set);
}
