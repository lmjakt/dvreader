#include "blobClassifier.h"
#include <iostream>

using namespace std;

BlobClassifier::BlobClassifier()
{
  classParameters.insert(BlobMapper::SUM);
  classParameters.insert(BlobMapper::VOLUME);
  classParameters.insert(BlobMapper::MAX);
  classParameters.insert(BlobMapper::MEAN);
  classParameters.insert(BlobMapper::SURFACE);
}

BlobClassifier::~BlobClassifier(){
  clearData();
}

void BlobClassifier::setSuperBlobs(vector<SuperBlob*>& sblobs){
  clearData();
  for(uint i=0; i < sblobs.size(); ++i){
    uint membership = sblobs[i]->membership;
    for(uint j=0; j < sblobs[i]->blobs.size(); ++j){
      blobs[ sblobs[i]->blobs[j].mapper ][ membership ].push_back(sblobs[i]->blobs[j].b);
      bmwMap[ sblobs[i]->blobs[j].mapper_id ] = sblobs[i]->blobs[j].mapper;
    }
  }
  // and then make the requisite number of mappers:
  map<BlobMapper*, map<uint, vector<blob*> > >::iterator it;
  for( it=blobs.begin(); it != blobs.end(); ++it)
    classifiers[(*it).first ] = new ND_Classifier();
  
}

void BlobClassifier::setParameters(set<BlobMapper::Param> params){
  classParameters = params;
  // redo the training? 
  trainClassifiers();
}

void BlobClassifier::clearData(){
  while(classifiers.size()){
    delete (*classifiers.begin()).second;
    classifiers.erase(classifiers.begin());
  }
  blobs.clear();
  bmwMap.clear();
}

void BlobClassifier::trainClassifiers(){
  for(map<BlobMapper*, ND_Classifier*>::iterator it=classifiers.begin();
      it != classifiers.end(); ++it)
    trainClassifier((*it).first);
}

void BlobClassifier::trainClassifier(BlobMapper* mapper){
  if(!classifiers.count(mapper) || !blobs.count(mapper)){
    cerr << "BlobClassifier::trainClassifier : unable to find mapper or blob map for : " << mapper << endl;
    return;
  }
  vector<int> classes;
  uint size=0;
  map<uint, vector<blob*> >& b = blobs[mapper];
  map<uint, vector<blob*> >::iterator it;
  for(it = b.begin(); it != b.end(); ++it)
    size += (*it).second.size();
  
  classes.resize(size);
  float* data = new float[ classParameters.size() * size];

  uint c = 0;
  for(it = b.begin(); it != b.end(); ++it){
    for(vector<blob*>::iterator vit=(*it).second.begin();
	vit != (*it).second.end(); ++vit){
      classes[c] = (*it).first;
      setTrainData(data + c * classParameters.size(), (*vit), mapper);
      ++c;
    }
  }
  // and here pass the data on to the appropriate classifier..
  classifiers[mapper]->setTrainingSet(classes, data, size, classParameters.size(), true);
}

void BlobClassifier::setTrainData(float* data, blob* b, BlobMapper* bm){
  for(set<BlobMapper::Param>::iterator it = classParameters.begin();
      it != classParameters.end(); ++it)
    {
      (*data) = bm->getParameter(b, (*it));
      ++data;
    }
}
