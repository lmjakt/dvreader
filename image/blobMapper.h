#ifndef BLOBMAPPER_H
#define BLOBMAPPER_H

#include "volumeMask.h"
#include "imageAnalyser.h"
#include <vector>
#include <map>
#include <set>



struct blob {
    blob(){
	min = max = 0;
	peakPos = 0;
    }
    std::vector<unsigned long> points;
    std::vector<float> values;
    std::vector<bool> surface; 
    float min, max;
    int max_x, min_x, max_y, min_y, max_z, min_z;
    unsigned long peakPos;
};

class BlobMapper
{
 public:
    BlobMapper(ImageAnalyser* ia);
    ~BlobMapper();

    std::set<blob*> mapBlobs(float minEdge, unsigned int wi, int window, bool eatContained=false);
    
 private:
    std::map<unsigned long, blob*> blobMap;
    std::set<blob*> blobs;
    int width, height, depth;
    unsigned int waveIndex;
    ImageAnalyser* image;
    VolumeMask* vMask; // I may not need it here.. 

    // and some internal functions to facilitate the mapping procedure.d
    void makeBlob(int x, int y, int z, int w);
    void extendBlob(int x, int y, int z, blob* b, int w); // make this recursive maybe.
    bool isSurface(int x, int y, int z, blob* b, bool tight=false);
    void mergeBlobs(blob* newBlob, blob* oldBlob);
    void eatContainedBlobs();
    void eatContainedBlobs(blob* b);
    void eatContents(blob* b, VolumeMask* vm, int x, int y, int z);
    void finaliseBlobs();
    void finaliseBlob(blob* b);


    // No error checking. 
    float value(int x, int y, int z){
	float v=0;
	image->point(v, x, y, z, waveIndex);
	return(v);
    }
    unsigned long linear(int x, int y, int z){
	return((unsigned long)z * ((unsigned long)width * (unsigned long)height) + 
	       ((unsigned long)y * (unsigned long)width) + (unsigned long)x);
    }
    void toVol(unsigned long p, int& x, int& y, int& z){
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

};

#endif
