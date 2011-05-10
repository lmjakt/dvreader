#include "BlobModelSet.h"
#include "../image/blobModel.h"
#include "blob_mt_mapper.h"
#include "../image/blob.h"
#include <iostream>

using namespace std;

BlobModelSet::BlobModelSet(unsigned int set_id, int xy_radius, int z_radius)
{
  super_id = set_id;
  xyRadius = xy_radius;
  zRadius = z_radius;
}

BlobModelSet::~BlobModelSet()
{
  for(map<unsigned int, BlobModel*>::iterator it=models.begin(); it != models.end(); ++it)
    delete (*it).second;
}

// all blob_sets should have been derived from a specific super set.
// (and have passed pre
void BlobModelSet::trainModels(vector<blob_set>& bsets)
{
  if(!bsets.size())
    return;
  deleteModels();
  map<Blob_mt_mapper*, vector<blob*> > mblobs = arrangeBlobs(bsets);
  for(map<Blob_mt_mapper*, vector<blob*> >::iterator it=mblobs.begin(); it != mblobs.end(); ++it){
    unsigned int map_id = (*it).first->mapId();
    if(!models.count(map_id))
      models[ map_id ] = new BlobModel(xyRadius, zRadius);
    trainModel( models[map_id], (*it).second, (*it).first );
  }
}

void BlobModelSet::assessBlobs(vector<blob_set>& bsets)
{
  map<Blob_mt_mapper*, vector<blob*> > mblobs = arrangeBlobs(bsets);
  for(map<Blob_mt_mapper*, vector<blob*> >::iterator it=mblobs.begin(); it !=mblobs.end(); ++it){
    unsigned int map_id = (*it).first->mapId();
    if(!models.count(map_id))
      continue;
    (*it).first->blob_model_correlations(models[map_id], (*it).second);
  }
}

void BlobModelSet::trainModel(BlobModel* model, std::vector<blob*>& blobs, Blob_mt_mapper* mapper)
{
  mapper->setImageStack(true); // subtract background is true
  mapper->addToBlobModel(model, blobs);
  mapper->freeImageStack();
}

map<Blob_mt_mapper*, vector<blob*> > BlobModelSet::arrangeBlobs(vector<blob_set>& bsets)
{
  map<Blob_mt_mapper*, vector<blob*> > mblobs;
  for(unsigned int i=0; i < bsets.size(); ++i){
    if(!checkIds(bsets[i]))
      continue;
    for(unsigned int j=0; j < bsets[i].size(); ++j)
      mblobs[ bsets[i].bm(j) ].push_back( bsets[i].b(j) );
  }
  return(mblobs);
}

bool BlobModelSet::checkIds(blob_set& bs){
  unsigned int sum = 0;
  vector<unsigned int> ids = bs.ids();
  for(unsigned int i=0; i < ids.size(); ++i)
    sum += ids[i];
  if(sum != super_id){
    cerr << "BlobModelSet::checkIds attempt to incorporate incorrect blob set: " << sum 
	 << " != " << super_id << endl;
    return(false);
  }
  return(true);
}

void BlobModelSet::deleteModels()
{
  for(map<unsigned int, BlobModel*>::iterator it=models.begin(); it != models.end(); ++it)
    delete (*it).second;
  models.clear();
}
