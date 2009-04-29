#include "imageData.h"
#include <iostream>
#include <math.h>

using namespace std;

ImageData::ImageData(FileSet* fs, uint interpolate){
    data = fs;
    interp = interpolate;

    volume = new VolumeCache();    // these are empty and will be destroyed the first time that we try to get something from them.. 
    interpBuffer = 0;
    cache_size = pCache * 40 / (interp * interp * interp);

    // note that this is the size of the interpBuffer the volumeCache will be interp^3 times smaller.

    // treat this a bit seriously. Seems to be completely screwed up in ImageAnalyser ? not sure 

    cache_depth = data->sectionNo()/interp;
//    cache_depth = cache_depth > data->sectionNo()/interp ? data->sectionNo()/interp : cache_depth;

    // 
    cache_width = data->pwidth() / interp;
    cache_height = data->pheight() / interp;
//    cache_height = cache_size / (cache_width * cache_depth ); 

    // make sure that the cache dimensions are not larger than the real dimensions.
    //cache_width = cache_width > data->pwidth()/interp ? data->pwidth()/interp : cache_width;
    //cache_height = cache_height > data->pheight()/interp ? data->pheight()/interp : cache_height;

    rc_width = cache_width;
    rc_height = cache_height;
    rc_depth = cache_depth;

    cache_size = cache_width * cache_height * cache_depth;
    cout << "ImageData created with cache size : " << cache_size << endl
	 << "                       cache_width: " << cache_width << endl
	 << "                      cache_height: " << cache_height << endl;
}

ImageData::~ImageData(){
    delete volume;
    delete interpBuffer;
}


void ImageData::dims(int& w, int& h, int& d){
    w = data->pwidth() / interp;
    h = data->pheight() / interp;
    d = data->sectionNo() / interp;
}

void ImageData::dims(unsigned int& w, unsigned int& h, unsigned int& d){
    w = (unsigned int)data->pwidth()/interp;
    h = (unsigned int)data->pheight()/interp;
    d = (unsigned int)data->sectionNo()/interp;
}


bool ImageData::point(float& p, int xp, int yp, int zp, unsigned int wi){
    bool ok = false;
    if( !(ok = volume->point(p, xp, yp, zp, wi) )){
	readData(xp, yp, zp, wi);
	ok = volume->point(p, xp, yp, zp, wi);
    }
    return(ok);
}

void ImageData::readData(int x, int y, int z, uint wi){
    if( rc_width != cache_width || rc_height != cache_height || rc_depth != cache_depth || !(volume->hasData()) )
	makeCache();
    
    int cw = cache_width * interp;
    int ch = cache_height * interp;
    int cd = cache_depth * interp;

    // and then work out the read positions. 
    int xb = (x * interp)  - (cache_width / 4);
    xb = (xb + cw) >= data->pwidth() ? (data->pwidth() - cw) : xb;
    xb = xb < 0 ? 0 : xb;
    
    int yb = (y * interp) - (cache_height / 4);
    yb = (yb + ch) >= data->pheight() ? (data->pheight() - ch) : yb;
    yb = yb < 0 ? 0 : yb;
    
    int zb = (z * interp) - (cache_depth / 4);
    zb = (zb + cd) >= data->sectionNo() ? (data->sectionNo() - cd) : zb;
    zb = zb < 0 ? 0 : zb;
    
    cout << "interp : " << interp << "  requesting pos: " << x << "," << y << "," << z << endl;
    cout << "cache made for point : " << x << "," << y << "," << z << "  using: " << xb << "->" << xb + cache_width 
	 << " , " << yb << "->" << yb + cache_height << " , " << zb << "->" << zb + cache_depth << endl;
    
    
    // if interpolate == 1 then we can read directly into the volume cache cache
    // otherwise we'll need to read into a buffer and then adjust the values from there
    float* readBuffer = volume->cacheData();
    if(interp > 1)
	readBuffer = interpBuffer;  // created in makeCache()
    memset((void*)readBuffer, 0, sizeof(float) * cache_size * interp * interp * interp);
    
    if( !(data->readToFloat(readBuffer, xb, yb, zb, cw, ch, cd, wi)) ){
	cerr << "ImageAnalyser::point  Unable to create new cache covering  " << xb << ", " << yb << ", " << zb << "   dims : " 
	     << cache_width << ", " << cache_height << ", " << cache_depth << endl;
	return;
    }
    volume->setCoords(xb/interp, yb/interp, zb/interp, wi);
    if(interp == 1)
	return;
    cout << "calling setInterpolatedData" << endl;
    volume->setInterpolatedData(readBuffer, interp);
    cout << "_______--------------------______________" << endl;
}


void ImageData::makeCache(){
    // we need to make a new volumeCache and possibly a new interpBuffer (if interP is larger than 1);
    cout << "making a cache .. eh? " << endl;
    delete volume;
    delete interpBuffer;
    interpBuffer = 0;     // if we have one, then delete, then don't have one we need it to be 0.

    float* newCache = new float[ cache_width * cache_height * cache_depth ];
    volume = new VolumeCache(newCache, 0, 0, 0, cache_width, cache_height, cache_depth, 0);
    
    if(interp > 1)
	interpBuffer = new float[ cache_width * cache_height * cache_depth * interp * interp * interp];
}
