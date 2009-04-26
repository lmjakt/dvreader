#include "blob.h"
#include <iostream>

using namespace std;

void blob::flatten(){
  flatten(this);
  deleteChildren();
}

void blob::flatten(blob* parentBlob){
    for(uint i=0; i < blobs.size(); ++i){
	parentBlob->points.insert(parentBlob->points.end(), blobs[i]->points.begin(), blobs[i]->points.end());
	parentBlob->values.insert(parentBlob->values.end(), blobs[i]->values.begin(), blobs[i]->values.end());
	parentBlob->surface.insert(parentBlob->surface.end(), blobs[i]->surface.begin(), blobs[i]->surface.end());
	blobs[i]->flatten(parentBlob);
    }
    // do not delete within this function as it is recursive.
    blobs.clear();
    blobs.resize(0);
}

void blob::size(uint& s){
    s += points.size();
    for(uint i=0; i < blobs.size(); ++i)
	blobs[i]->size(s);
    
}

void blob::childNo(unsigned int& c){
    c += blobs.size();
    for(uint i=0; i < blobs.size(); ++i)
	blobs[i]->childNo(c);
}
