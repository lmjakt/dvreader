#include "blob.h"

using namespace std;

void blob::flatten(blob* parentBlob){
    for(uint i=0; i < blobs.size(); ++i){
	parentBlob->points.insert(parentBlob->points.end(), blobs[i]->points.begin(), blobs[i]->points.end());
	parentBlob->values.insert(parentBlob->values.end(), blobs[i]->values.begin(), blobs[i]->values.end());
	parentBlob->surface.insert(parentBlob->surface.end(), blobs[i]->surface.begin(), blobs[i]->surface.end());
	blobs[i]->flatten(parentBlob);
	delete(blobs[i]);
    }
    blobs.resize(0);
}

void blob::size(unsigned int& s){
    s += points.size();
    for(uint i=0; i < blobs.size(); ++i)
	blobs[i]->size(s);
    
}
