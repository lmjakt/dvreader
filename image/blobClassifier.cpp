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
  cout << "before clearing data in the super blobs things " << endl;
  clearData();
  cout << "data cleared" << endl;
  for(uint i=0; i < sblobs.size(); ++i){
    cout << "|";
    uint membership = sblobs[i]->membership;
    for(uint j=0; j < sblobs[i]->blobs.size(); ++j){
      cout << ".";
      blobs[ sblobs[i]->blobs[j].mapper ][ membership ].push_back(sblobs[i]->blobs[j].b);
      bmwMap[ sblobs[i]->blobs[j].mapper_id ] = sblobs[i]->blobs[j].mapper;
    }
    cout << endl;
  }
  cout << "set super blobs blobs and bmwmap set up" << endl;
  // and then make the requisite number of mappers:
  map<BlobMapper*, map<uint, vector<blob*> > >::iterator it;
  for( it=blobs.begin(); it != blobs.end(); ++it)
    classifiers[(*it).first ] = new ND_Classifier();
  
  trainClassifiers();
}

void BlobClassifier::setParameters(set<BlobMapper::Param> params){
  classParameters = params;
  // redo the training? 
  cout << "calling trainClassifiers after thingy" << endl;
  trainClassifiers();
}

void BlobClassifier::removeBlobMapper(BlobMapper* bm){
  if(classifiers.count(bm)){
    delete classifiers[bm];
    classifiers.erase(bm);
  }
  blobs.erase(bm);
}

map<BlobMapper*, map<uint, vector<blob*> > > BlobClassifier::gBlobs(){
  return(blobs);
}

vector<BlobClassCounts> BlobClassifier::classifyBlobs(vector<BlobMapper*> mappers){
  vector<BlobClassCounts> classCounts;
  vector<int> classes;
  for(uint i=0; i < mappers.size(); ++i){
    float* classData = 0;
    float** dpntr = &classData;
    classCounts.push_back(classifyBlobs(mappers[i], dpntr, classes, true));
    // not sure how to use the whole data set at the moment, so let's just delete it
    cout << "Just before deleting classData " << endl;
    delete classData;

  }
  cout << "just before returning classcounts" << endl;
  return(classCounts);
}

void BlobClassifier::clearData(){
  cout << "classifiers size is : " << classifiers.size() << endl;
  while(classifiers.size()){
    cout << "deleting classifier : " << (*classifiers.begin()).second << endl;
    delete (*classifiers.begin()).second;
    cout << "deleted but not erased " << endl;
    classifiers.erase(classifiers.begin());
    cout << "classifier map position erased" << endl;
  }
  cout << "about to clear" << endl;
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

BlobClassCounts BlobClassifier::classifyBlobs(BlobMapper* mapper, float** classData, vector<int>& classes, bool normalise)
{
  BlobClassCounts counts(mapper);
  ND_Classifier* classifier = 0;
  for(map<BlobMapper*, ND_Classifier*>::iterator it=classifiers.begin();
      it != classifiers.end(); ++it){
    if((*it).first->info() == mapper->info())
      classifier = (*it).second;
  }
  if(!classifier){
    cerr << "BlobClassifier::classifyBlobs No appropriate classifier found" << endl;
    return(counts);
  }
  set<blob*> b = mapper->gBlobs();
  float* data = extractBlobData(b, mapper);
  //  cout << "BlobClassifier callling classify with a data set of size : " << b.size() << " x " << classParameters.size() << endl;
  (*classData) = classifier->classify(data, b.size(), classParameters.size(), classes, normalise);

  counts.counts = countClasses(*classData, b, classes, true);

  counts.class_super_counts = count_ClassSuperCounts(b);

  delete data;
  cout << "returning counts happily" << endl;
  return(counts);
}

float* BlobClassifier::extractBlobData(set<blob*> b, BlobMapper* bm){
  if(!b.size() || !classParameters.size()){
    cerr << "BlobClassifier::extractBlobData either empty set of classParameters or blobs" << endl;
    return(0);
  }
  float* data = new float[b.size() * classParameters.size()];
  float* dptr = data;
  for(set<blob*>::iterator it=b.begin(); it != b.end(); ++it){
    for(set<BlobMapper::Param>::iterator pit=classParameters.begin();
	pit != classParameters.end(); ++pit){
      (*dptr) = bm->getParameter((*it), (*pit));
      ++dptr;
    }
  }
  return(data);
}

// the following function is horrible. Rewrite if possible.
map<int, uint> BlobClassifier::countClasses(float* classData, set<blob*>& b, vector<int>& classes, bool setBlobs){
  map<int, uint> counts;
  if(!classes.size() || !b.size()){
    cerr << "BlobClassifier::countClasses empty data returning" << endl;
    return(counts);
  }
  float* lr = new float[classes.size()];
  uint r = 0;
  for(set<blob*>::iterator it=b.begin(); it != b.end(); ++it){
    float mean = 0;
    for(uint c=0; c < classes.size(); ++c){
      lr[c] = classData[ r * classes.size() + c];
      mean += lr[c];
    }
    for(uint c=0; c < classes.size(); ++c)
      lr[c] = log( lr[c] / mean);

    float lr_max = lr[0];
    int blobClass = classes[0];
    for(uint c=1; c < classes.size(); ++c){
      if(lr[c] > lr_max){
	lr_max = lr[c];
	blobClass = classes[c];
      }
    }

    if(!counts.count(blobClass))
      counts[blobClass] = 0;
    counts[blobClass]++;
    if(setBlobs){
      (*it)->b_class = blobClass;
      (*it)->class_lr = lr_max;
    }

    ++r;
  }

  delete lr;

  return(counts);
}

map<int, map<int, uint> > BlobClassifier::count_ClassSuperCounts(set<blob*>& b){
  map<int, map<int, uint> > counts;
  for(set<blob*>::iterator it=b.begin(); it !=b.end(); ++it){
    cout << "class : " << (*it)->b_class << endl;
    cout << "\tsuper : " << (*it)->super_class << endl;
    if(!counts[ (*it)->b_class ].count( (*it)->super_class ) )
      counts[ (*it)->b_class ][ (*it)->super_class ] = 0;
    counts[ (*it)->b_class ][ (*it)->super_class ]++;
  }
  return(counts);
}
