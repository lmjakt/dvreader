#ifndef VOLUMECACHE_H
#define VOLUMECACHE_H

#include <iostream>

enum Dimension {
    XDIM, YDIM, ZDIM
};


class VolumeCache {
    float* cache;
    unsigned int size;  // the size of the cache..
    int x, y, z, w, h, d;  // the logical positions of the cache..
    unsigned int waveIndex;
//    float waveLength;        // the waveLength represented by the data..

    public :
	VolumeCache(){
	cache = 0;
	x = y = z = 0;
	w = h = d = -1;   // this makes all requests false..
    }
    VolumeCache(float* c, int xb, int yb, int zb, int width, int height, int depth, unsigned int wi){
	cache = c;
	size = width * height * depth;
	x = xb;
	y = yb;
	z = zb;
	w = width;
	h = height;
	d = depth;
	waveIndex = wi;
//	waveLength = wl;
    }
    ~VolumeCache(){
	if(cache){
	    delete cache;
	}
    }
    void setInterpolatedData(float* source, unsigned int interp);
    void setCoords(int xb, int yb, int zb, unsigned int wi){
	x = xb; y = yb; z = zb;
	waveIndex = wi;
    }
    void setDims(int width, int height, int depth){
	w = width; h = height; d = depth;
    }
    float* cacheData(){
	return(cache);
    }
    bool hasData(){
	return( w != -1 );
    }
    // then some functions that give a line..
    bool line(float* line, int xb, int yb, int zb, int l, Dimension dim, unsigned int wi);   // returns false if not possible
    bool point(float& p, int xp, int yp, int zp, unsigned int wi){
	// inline function since it may be called a lot.. 
	if(wi != waveIndex){
	    std::cout << "point() wrong wi requested: " << wi << std::endl;
	    return(false);
	}
	if(!(xp >= x && xp < x + w && yp >= y && yp < y + h && zp >= z && zp < z + d) ){
	    std::cout << "VolumeCache::point point not in cache : " << xp << ", " << yp << ", " << zp 
		      << " x,y,z : " << x << "," << y << "," << z << "  dims : " << w << "," << h << "," << d << std::endl;
	    return(false);
	}
	p = cache[(zp - z) * w * h + (yp - y) * w + (xp - x)];
	return(true);
    }
};



#endif
