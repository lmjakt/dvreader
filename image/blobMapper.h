#ifndef BLOBMAPPER_H
#define BLOBMAPPER_H

#include "volumeMask.h"
#include "imageAnalyser.h"
#include "blob.h"
#include <vector>
#include <map>
#include <set>


class BlobMapper
{
 public:
    BlobMapper(ImageAnalyser* ia);
    ~BlobMapper();

    std::set<blob*> mapBlobs(float minEdge, unsigned int wi, int window, bool eatContained, bool eat_neighbors);
    
 private:
    struct NeighborInfo {
	std::vector<float> values;
	float sum;
	float peakValue;
	NeighborInfo(){
	    sum = 0; peakValue = 0;
	}
	void addPoint(float v){
	    values.push_back(v);
	    sum += v;
	    peakValue = peakValue < v ? v : peakValue;
	}
    };

    std::map<off_set, blob*> blobMap;
    std::set<blob*> blobs;
    int width, height, depth;
    unsigned int waveIndex;
    ImageAnalyser* image;
    VolumeMask* vMask; // I may not need it here.. 

    // and some internal functions to facilitate the mapping procedure.d
    void makeBlob(int x, int y, int z, int w);
    void extendBlob(int x, int y, int z, blob* b, int w); // make this recursive maybe.
    bool isSurface(int x, int y, int z, blob* b, bool tight=false);
    void mergeBlobs(blob* newBlob, blob* oldBlob);        // new blob is deleted, so iterators must point to old blob.
    void eatContainedBlobs();
    void eatContainedBlobs(blob* b);
    void eatContents(blob* b, VolumeMask* vm, int x, int y, int z);
    void eatNeighbors();
    void eatNeighbors(blob* b);
    std::vector<off_set> findNeighbors(blob* b, off_set p);

    void finaliseBlobs();
    void finaliseBlob(blob* b);


    // No error checking. 
    float value(int x, int y, int z){
	float v=0;
	image->point(v, x, y, z, waveIndex);
	return(v);
    }
    off_set linear(int x, int y, int z){
	return((off_set)z * ((off_set)width * (off_set)height) + 
	       ((off_set)y * (off_set)width) + (off_set)x);
    }
    void toVol(off_set p, int& x, int& y, int& z){
	z = p / (width * height);
	y = (p % (width * height)) / width;
	x = p % width;
    }
    // no bounds checking. Do elsewhere.
    bool sameBlob(int x, int y, int z, blob* b){
	if(!vMask->mask(x, y, z))
	    return(false);
	return( blobMap[ linear(x, y, z) ] == b);
    }
    bool differentBlob(int x, int y, int z, blob* b){
	if(x >= 0 && y >=0 & z >= 0 && x < width && y < height && z < depth){
	    if(!vMask->mask(x, y, z))
		return(false);
	    return(blobMap[linear(x, y, z)] != b);
	}
	return(false);
    }
};

#endif
