#include "BlobMerger.h"
#include "blob_mt_mapper.h"
#include "../image/blob.h"
#include "stack_info.h"
#include <iostream>

using namespace std;

BlobMerger::BlobMerger()
{
}

BlobMerger::~BlobMerger()
{
  for(unsigned int i=0; i < maps.size(); ++i)
    delete []maps[i];
}

vector<blob_set> BlobMerger::mergeBlobs(vector<Blob_mt_mapper*> bl_mappers, vector<ChannelOffset> ch_offsets, unsigned int r)
{
  vector<blob_set> blob_sets;
  channelOffsets = ch_offsets;
  if(!bl_mappers.size() || bl_mappers.size() != channelOffsets.size()){
    cerr << "BlobMerger::mergeBlobs received empty bl_mappers" << endl;
    return(blob_sets);
  }
  mappers = bl_mappers;
  pos = mappers[0]->pos;
  for(unsigned int i=1; i < mappers.size(); ++i){
    if(mappers[i]-> pos != pos){
      cerr << "BlobMerger::mergeBlobs trying to add mapper with differing dimensions" << endl;
      return(blob_sets);
    }
  }
  radius = r;
  initOffsets();
  initMaps();
  
  return(merge());
}

void BlobMerger::initOffsets()
{
  offsets.resize(0);
  int d = 1 + radius * 2;
  offsets.reserve(d * d * d);
  for(int dz=-radius; dz <= +radius; ++dz){
    for(int dy=-radius; dy <= +radius; ++dy){
      for(int dx=-radius; dx <= +radius; ++dx){
	offsets.push_back(dz * pos.w * pos.h + dy * pos.w + dx);
      }
    }
  }
}


vector<int> BlobMerger::adjustOffsets(unsigned int map_id)
{
  if(map_id >= mappers.size())
    return(offsets);
  vector<int> adjusted(offsets.size());
  for(unsigned int i=0; i < offsets.size(); ++i){
    adjusted[i] = offsets[i] + 
      (channelOffsets[map_id].z() * pos.w * pos.h) + 
      (channelOffsets[map_id].y() * pos.w) + channelOffsets[map_id].x();
  }
  return(adjusted);
}

void BlobMerger::initMaps()
{
  for(unsigned int i=0; i < maps.size(); ++i)
    delete []maps[i];
  
  maps.resize(mappers.size());
  for(unsigned int i=0; i < mappers.size(); ++i){
    maps[i] = new blob*[pos.d * pos.h * pos.w];
    memset((void*)maps[i], 0, sizeof(blob*) * pos.w * pos.h * pos.d);
    initMap(mappers[i], maps[i]);
  }
}

void BlobMerger::initMap(Blob_mt_mapper* mapper, blob** map)
{
  int size = pos.w * pos.h * pos.d;
  for(multimap<unsigned int, blob*>::iterator it = mapper->blobs.begin(); 
      it != mapper->blobs.end(); ++it){
    if((int)(*it).second->peakPos >= size){
      cerr << "BlobMerger::initMap peakPos too high: " << (*it).second->peakPos << " > " << size << endl;
      continue;
    }
    map[ (*it).second->peakPos ] = (*it).second;
  }
}
 
vector<blob*> BlobMerger::getNeighborBlobs(unsigned int map_id, int p)
{
  vector<blob*> nb;
  if(map_id >= maps.size())
    return(nb);
  int size = pos.w * pos.h * pos.d;
  vector<int> adjustedOffsets = adjustOffsets(map_id);
  for(unsigned int i=0; i < adjustedOffsets.size(); ++i){
    int o = adjustedOffsets[i] + p;
    if(o < 0 || o >= size)
      continue;
    if(maps[map_id][o])
      nb.push_back(maps[map_id][o]);
  }
  return(nb);
}

bool BlobMerger::isComplete(vector<blob*> bv)
{
  set<blob*> b;
  b.insert(bv.begin(), bv.end());
  for(set<blob*>::iterator it=b.begin(); it != b.end(); ++it){
    int pos = (*it)->peakPos;
    for(unsigned int i=0; i < maps.size(); ++i){
      vector<blob*> nb = getNeighborBlobs(i, pos);
      for(unsigned int j=0; j < nb.size(); ++j){
	if(!b.count(nb[j]))
	  return(false);
      }
    }
  }
  return(true);
}

vector<blob_set> BlobMerger::merge()
{
  vector<blob_set> bsets;
  set<blob*> mergedBlobs;

  for(unsigned int i=0; i < mappers.size(); ++i){
    for(multimap<unsigned int, blob*>::iterator it=mappers[i]->blobs.begin();
	it != mappers[i]->blobs.end(); ++it){
      if(mergedBlobs.count( (*it).second ))
	continue;
      blob_set bs(pos.x, pos.y, pos.z);
      bs.push((*it).second, mappers[i]->map_id, mappers[i]);
      mergedBlobs.insert( (*it).second );
      for(unsigned j=0; j < mappers.size(); ++j){
	if(i != j){
	  vector<blob*> nb = getNeighborBlobs(j, (*it).second->peakPos);
	  if(nb.size() > 1)
	    bs.addError(blob_set::MULTI);
	  for(uint k=0; k < nb.size(); ++k){
	    if(mergedBlobs.count(nb[k])){
	      bs.addError(blob_set::INCOMPLETE);
	      continue;
	    }
	    bs.push(nb[k], mappers[j]->map_id, mappers[j]);
	    mergedBlobs.insert(nb[k]);
	  }
	}
      }
      if(!isComplete(bs.b()))
	bs.addError(blob_set::EXTENSIBLE);
      bsets.push_back(bs);
    }
  }
  return(bsets);
}

	

