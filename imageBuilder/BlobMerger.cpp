#include "BlobMerger.h"
#include "blob_mt_mapper.h"
#include "../image/blob.h"
#include "stack_info.h"
#include <iostream>
#include <math.h>

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
  offset_dists.resize(0);
  int d = 1 + radius * 2;
  offsets.reserve(d * d * d);
  offset_dists.reserve(d * d * d);
  for(int dz=-radius; dz <= +radius; ++dz){
    for(int dy=-radius; dy <= +radius; ++dy){
      for(int dx=-radius; dx <= +radius; ++dx){
	offsets.push_back(dz * pos.w * pos.h + dy * pos.w + dx);
	offset_dists.push_back( sqrtf( (dz*dz) + (dy*dy) + (dz*dz) ) );
      }
    }
  }
  maxBlobDistance = offset_dists[0]; // as this is for -dz, -dy, -dx
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
 
// rewrite this so that it gives only the closest or the strongest blob
// more than one neighbour makes no sense
vector<blob*> BlobMerger::getNeighborBlobs(unsigned int map_id, int p)
{
  vector<blob*> nb;
  float dist = 2 * offset_dists[0]; // this assumes that offsets start from -r,-r,-r
  if(map_id >= maps.size())
    return(nb);
  int size = pos.w * pos.h * pos.d;
  vector<int> adjustedOffsets = adjustOffsets(map_id);
  for(unsigned int i=0; i < adjustedOffsets.size(); ++i){
    int o = adjustedOffsets[i] + p;
    if(o < 0 || o >= size)
      continue;
    if(!maps[map_id][o]) 
      continue;
    if(!nb.size()){
      nb.push_back(maps[map_id][o]);
      dist = offset_dists[i];
      continue;
    }
    if(offset_dists[i] <= dist){
      if(offset_dists[i] < dist || nb[0]->max < maps[map_id][o]->max){
	 nb[0] = maps[map_id][o];
	 dist = offset_dists[i];
      }
    }
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
  map<blob*, set<blob*> > blob_links;
  for(unsigned int i=0; i < mappers.size(); ++i){
    for(multimap<unsigned int, blob*>::iterator it=mappers[i]->blobs.begin();
   	it != mappers[i]->blobs.end(); ++it){
      set<blob*> links;
      for(unsigned j=0; j < mappers.size(); ++j){
 	if(i != j){
 	  vector<blob*> nb = getNeighborBlobs(j, (*it).second->peakPos);
	  links.insert(nb.begin(), nb.end());
	}
      }
      links = checkNeighbours( (*it).second, links );
      links.insert( (*it).second );
      blob_links.insert(make_pair((*it).second, links));  // this will insert duplicates & triplicates
    }
  }
  // No error codes set..
  return( collapseDuplicates(blob_links) );
	  

  // vector<blob_set> bsets;
  // set<blob*> mergedBlobs;

  // for(unsigned int i=0; i < mappers.size(); ++i){
  //   for(multimap<unsigned int, blob*>::iterator it=mappers[i]->blobs.begin();
  // 	it != mappers[i]->blobs.end(); ++it){
  //     if(mergedBlobs.count( (*it).second ))
  // 	continue;
  //     blob_set bs(pos.x, pos.y, pos.z);
  //     bs.push((*it).second, mappers[i]->map_id, mappers[i]);
  //     mergedBlobs.insert( (*it).second );
  //     for(unsigned j=0; j < mappers.size(); ++j){
  // 	if(i != j){
  // 	  vector<blob*> nb = getNeighborBlobs(j, (*it).second->peakPos);
  // 	  if(nb.size() > 1)
  // 	    bs.addError(blob_set::MULTI);
  // 	  for(uint k=0; k < nb.size(); ++k){
  // 	    if(mergedBlobs.count(nb[k])){
  // 	      bs.addError(blob_set::INCOMPLETE);
  // 	      continue;
  // 	    }
  // 	    bs.push(nb[k], mappers[j]->map_id, mappers[j]);
  // 	    mergedBlobs.insert(nb[k]);
  // 	  }
  // 	}
  //     }
  //     if(!isComplete(bs.b()))
  // 	bs.addError(blob_set::EXTENSIBLE);
  //     bsets.push_back(bs);
  //   }
  // }
  // return(bsets);
}

// Note that the set<blob*> pointed to by blob* needs to include blob* itself
// for this function to work.
// 
vector<blob_set> BlobMerger::collapseDuplicates(map<blob*, set<blob*> >& blob_links)
{
  vector<blob_set> bsets;
  map<blob*, set<blob*> >::iterator map_it;
  while(blob_links.size()){
    map_it = blob_links.begin();
    blob* seed = (*map_it).first;
    set<blob*> nbs = (*map_it).second;
    blob_links.erase(map_it);
    blob_set bs(pos.x, pos.y, pos.z);
    bs.push( seed, seed->mapper->map_id, seed->mapper );
    for(set<blob*>::iterator it=nbs.begin(); it != nbs.end(); ++it){
      bs.push( (*it), (*it)->mapper->map_id, (*it)->mapper );
      if( blob_links.count(*it) && sets_identical(nbs, blob_links[ (*it) ]) )
	  blob_links.erase( (*it) );
    }
    bsets.push_back(bs);
  }
  return(bsets);
}

// this function could easily be written as a template function
bool BlobMerger::sets_identical(set<blob*>& a, set<blob*>& b)
{
  if(a.size() != b.size())
    return(false);
  for(set<blob*>::iterator it=a.begin(); it != a.end(); ++it){
    if(!b.count(*it)) return false;
  }
  return(true);
}

// if two members of the set have more than max distance remove the
// one that is more distant.. (or remove both..)
// nbs should not contain seed, though if it does it shouldn't matter
set<blob*> BlobMerger::checkNeighbours(blob* seed, set<blob*> nbs)
{
  if(nbs.size() < 2)
    return(nbs);
  set<blob*>::iterator it1;
  set<blob*>::iterator it2;
  set<blob*> blobs_to_remove;
  for(it1=nbs.begin(); it1 != nbs.end(); ++it1){
    it2 = it1;
    ++it2;
    while(it2 != nbs.end()){
      if( blobDistance( (*it1), (*it1) ) > maxBlobDistance ){
	blobs_to_remove.insert(*it1);
	blobs_to_remove.insert(*it2);
	//	blob* r_blob = blobDistance( seed, (*it1)) > blobDistance( seed, (*it2) ) ? (*it1) : (*it2);
	//blobs_to_remove.insert(r_blob);
      }
      ++it2;
    }
  }
  for(set<blob*>::iterator it=blobs_to_remove.begin(); it != blobs_to_remove.end(); ++it)
    nbs.erase(*it);
  return(nbs);
}

float BlobMerger::blobDistance(blob* a, blob* b)
{
  if(!mappers.size()){
    cerr << "BlobMerger::pixelDistance called but no mappers specified" << endl;
    return(0);
  }
  int ax, ay, az, bx, by, bz;
  mappers[0]->toVol(a->peakPos, ax, ay, az);
  mappers[0]->toVol(b->peakPos, bx, by, bz);
  return( sqrtf( float( (ax-bx)*(ax-bx) + (ay-by)*(ay-by) + (az-bz)*(az-bz) ) ) );
}
