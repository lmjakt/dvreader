#include "volumeCache.h"
#include <string.h>
#include <iostream>

using namespace std;

// Note that source needs to have dimensions that are interp * dims
void VolumeCache::setInterpolatedData(float* source, unsigned int interp){
    memset((void*)cache, 0, sizeof(float) * w * h * d);
    float div = float(interp * interp * interp);
    int sw = w * interp;
    int sh = h * interp;
    int sd = d * interp;
    for(int dz=0; dz < sd; ++dz){
	for(int dy=0; dy < sh; ++dy){
	    for(int dx=0; dx < sw; ++dx){
		cache[ (dz/interp) * (w * h) + (dy/interp) * w + (dx/interp) ]
		    += (source[ dz * sh * sw + dy * sw + dx] / div);
	    }
	}
    }
}


bool VolumeCache::line(float* line, int xb, int yb, int zb, int l, Dimension dim, unsigned int wi){
    // first check if the starting point is within the area covered by the cache..
    bool ok = false;
    if(wi != waveIndex){
	return(ok);
    }
    if(!(xb >= x && xb < x + w && yb >= y && yb < y + h && zb >= z && zb < z + d)){
//	cout << "VolumeCache doesn't contain line with point " << xb << ", " << yb << ", " << zb << endl;
	return(ok);
    }
    if(l <= 0){
	cerr << "VolumeCache::simpleLine negative or 0 length line requested : " << endl;
	return(ok);
    }
    switch(dim){
	case XDIM :
	    if(xb + l <= x + w){
		memcpy((void*)line, cache + (zb-z) * h * w +  (yb - y) * w + (xb - x), sizeof(float) * l);
		ok = true;
	    }
	    break;
	case YDIM :
	    if(yb + l <= y + h){
		for(int i=0; i < l; ++i){
		    *line = cache[ (zb - z) * w * h + (yb + i - y) * w + (xb - x) ];
		    ++line;
		}
		ok = true;
	    }
	    break;
	case ZDIM :
	    if(zb + l <= z + d){
		for(int i=0; i < l; ++i){
		    *line = cache[ (zb + i - z) * w * h + (yb - y) * w + (xb - x) ];
		    ++line;
		}
		ok = true;
	    }
    }
    return(ok);
}
