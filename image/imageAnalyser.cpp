//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#include "imageAnalyser.h"
#include "threeDPeakFinder.h"
#include "../spotFinder/perimeter.h"
#include <iostream>
#include <string.h>
#include <math.h>
#include <time.h>
#include <values.h>
#include <stdio.h>
#include <algorithm>

using namespace std;

ImageAnalyser::ImageAnalyser(FileSet* fs){
    data = fs;
    area_cache = new VolumeCache();
    x_z_slice_cache = new VolumeCache();
    volume_cache = new VolumeCache();    // these are empty and will be destroyed the first time that we try to get something from them.. 
    cache_size = processorCache * 2;
//    cache_size = processorCache / 2;

    // then make sure that things are ok.. 
    area_cache_width = (data->pwidth() * data->pwidth()) <= cache_size ? data->pwidth() : 1 + (cache_size / data->pheight());   // make sure that it doesn't get too big..
    
    // the x_z slice is a bit more cpmplicated.. 
    x_z_slice_cache_width = (data->pwidth() * data->sectionNo()) <= cache_size ? data->pwidth() : 1 + (cache_size / (data->pwidth() * data->sectionNo()));
    x_z_slice_cache_height = cache_size / (x_z_slice_cache_width * data->sectionNo());
    x_z_slice_cache_height = x_z_slice_cache_height > 0 ? x_z_slice_cache_height : 1;

    // volume_cache should be more like random access.. 

    // if my cache is huge and my image is small I might end up with cache widths that are too big.. 
    area_cache_width = area_cache_width > data->pwidth() ? data->pwidth() : area_cache_width;
    x_z_slice_cache_width = x_z_slice_cache_width > data->pwidth() ? data->pwidth() : x_z_slice_cache_width;
    x_z_slice_cache_height = x_z_slice_cache_height > data->pheight() ? data->pheight() : x_z_slice_cache_height;
 
//    vol_cache_width = (2 * processorCache) / (min_point_height * min_point_depth);
    // vol_cache_width = data->pwidth();
    // vol_cache_width = vol_cache_width > data->pwidth() ? data->pwidth() : vol_cache_width;
    // vol_cache_width = vol_cache_width <= (min_point_height * 2) ? min_point_height * 2 : vol_cache_width;

    cout << "ImageAnalyser::ImageAnalyser construction complete image proportions are " << data->pwidth() << ", " << data->pheight() << ", " << data->sectionNo()
	 << "\ncache size are area_cache width :  " << area_cache_width
	 << "\nslice cache width and height    :  " << x_z_slice_cache_width << ", " << x_z_slice_cache_height << endl; 
    //    cout <<  "vol cache width                  :  " << vol_cache_width << ", " << min_point_height << ", " << min_point_depth << endl;
    

}


ImageAnalyser::~ImageAnalyser(){
    delete area_cache;
    delete x_z_slice_cache;
    delete volume_cache;
}

vector<vector<float> > ImageAnalyser::x_line(int y_pos, int z_pos){
    // first get the relevant information from thingy ..
    int l = data->pwidth();
    vector<float> channels = data->channels();
    vector<vector<float> > lines(channels.size());
    float* line = new float[l];
    memset((void*)line, 0, l * sizeof(float));
    cout << "ImageAnalyser::x_line pos : " << y_pos << ", " << z_pos << "  length : " << l << endl;
    for(uint i=0; i < channels.size(); i++){
	cout << "doing thing for channel : " << i << "  wavelength : " << channels[i] << endl;
	// anyway, let's copy the line ..
	lines[i].resize(l);
	if(!simpleLine(line, 0, y_pos, z_pos, l, XDIM, i)){
//	if(!simpleLine(line, 0, y_pos, z_pos, l, XDIM, channels[i])){
	    cerr << "ImageAnalyser unable to get the line from thingy.. " << endl;
	    // set line to 0. 
	    memset((void*)&lines[i][0], 0, sizeof(float) * l);
	}else{
	    memcpy((void*)&lines[i][0], line, sizeof(float) * l);
	}
    }
    delete line;
    cout << "returning x_lines" << endl;
    return(lines);
}


// this is just a wrapper function, -it provides the given area for all of the channels...
vector<float*> ImageAnalyser::mip_areas(int xb, int yb, int width, int height){
    vector<float*> areas;
    xb = xb < 0 ? 0 : xb;
    yb = yb < 0 ? 0 : yb;
    
    width = xb + width > data->pwidth() ? width + (data->pwidth() - (xb + width)) : width;
    height = yb + height > data->pheight() ? height + (data->pheight() - (yb + height)) : height;
    if(width <= 0 || height <= 0){
	cerr << "ImageAnalyser::mip_area 0 or negative width or height specified" << endl;
	return(areas);
    }

    // and then ..
    vector<float> channels = data->channels();
    areas.resize(channels.size());
    for(uint i=0; i < areas.size(); i++){
	areas[i] = new float[width * height];
	data->readToFloatPro(areas[i], xb, width, yb, height, i);
    }
    return(areas);
}

// and a wrapper for the above so that we don't have to specify the size of the image.. 
vector<float*> ImageAnalyser::mip_areas(int& width, int& height){
    width = data->pwidth();
    height = data->pheight();
    return(mip_areas(0, 0, width, height));
}

void ImageAnalyser::blur(float* source, float* dest, uint w, uint h, int r, double sigma, double order){
    // square thingy, rather than otherwise.. so..
    cout << "ImageAnalyser blur" << endl;
    if(r < 0){
	cerr << "ImageAnalyser::blur radius is less than 0, this is not good  " << r << endl;
	r = -r;
    }
    memset((void*)dest, 0, sizeof(float) * w * h);
    // and then just go through all the points.. 
    double m = 1.0 / sqrt(double( (r*r) + (r*r) ) );  // this is the maximum distance.. 
    for(int y=0; y < (int)h; ++y){
	cout << y << endl;
	for(int x=0; x < (int)w; ++x){
	    cout << ".";
	    // and then do a square blur .. 
	    int yb = (y - r) < 0 ? 0 : (y - r);
	    int ye = (y + r) <= (int)h ? (y + r) : (int)h - 1;
	    int xb = (x - r) < 0 ? 0 : (x - r);
	    int xe = (x + r) <= (int)w ? (x + r) : (int)w - 1;
	    for(int dy=yb; dy <= ye; ++dy){
		for(int dx=xb; dx <= xe; dx++){
		    double d = sqrt((double) ((dy - y)*(dy - y) + (dx - x)*(dx - x)));
		    dest[y * w + x] += source[dy * w + dx] - (source[dy * w + dx] * d * m);  // a cone blur (avoids calling pow);
//		    dest[y * w + x] += source[dy * w + dx] * pow(order, -(d * d)/sigma);
		}
	    }
	}
	cout << endl;
    }
    // and that is it ... 
}

// and one for the projection taken directly from the file set..
vector<vector<float> > ImageAnalyser::mip_xline(int ypos){
    int l = data->pwidth();
    vector<float> channels = data->channels();
    vector<vector<float> > lines(channels.size());
    float* line = new float[l];
    memset((void*)line, 0, sizeof(float));
    for(uint i=0; i < channels.size(); i++){
	lines[i].resize(l);
	if(!data->readToFloatPro(line, 0, l, ypos, 1, i)){    /// bit bad since we are not setting up stuff well here..
	    memset((void*)&lines[i][0], 0, sizeof(float) * l);
	}else{
	    memcpy((void*)&lines[i][0], line, sizeof(float) * l);
	}
    }
    delete line;
    return(lines);
}

vector<vector<float> > ImageAnalyser::y_line(int x_pos, int z_pos){
    // first get the relevant information from thingy ..
    int l = data->pheight();
    vector<float> channels = data->channels();
    vector<vector<float> > lines(channels.size());
    float* line = new float[l];
    memset((void*)line, 0, l * sizeof(float));
    cout << "ImageAnalyser::y_line : " << x_pos << ", " << z_pos << "  length : " << l << endl;
    for(uint i=0; i < channels.size(); i++){
	// anyway, let's copy the line ..
	cout << "\tchannel : " << i << "  wavelength " << channels[i] << endl;
	lines[i].resize(l);
	if(!simpleLine(line, x_pos, 0, z_pos, l, YDIM, i)){
//	if(!simpleLine(line, x_pos, 0, z_pos, l, YDIM, channels[i])){
	    cerr << "ImageAnalyser unable to get the line from thingy.. " << endl;
	    // set line to 0. 
	    memset((void*)&lines[i][0], 0, sizeof(float) * l);
	}else{
	    memcpy((void*)&lines[i][0], line, sizeof(float) * l);
	}
    }
    delete line;
    return(lines);
}

vector<vector<float> > ImageAnalyser::mip_yline(int xpos){
    int l = data->pheight();
    vector<float> channels = data->channels();
    vector<vector<float> > lines(channels.size());
    float* line = new float[l];
    memset((void*)line, 0, l * sizeof(float));
    for(uint i=0; i < channels.size(); i++){
	lines[i].resize(l);
	if(!data->readToFloatPro(line, xpos, 1, 0, l, i)){    /// bit bad since we are not setting up stuff well here..
	    memset((void*)&lines[i][0], 0, sizeof(float) * l);
	}else{
	    memcpy((void*)&lines[i][0], line, sizeof(float) * l);
	}
    }
    delete line;
    return(lines);
}

void ImageAnalyser::dims(int& w, int& h, int& d){
    w = data->pwidth();
    h = data->pheight();
    d = data->sectionNo();
}

void ImageAnalyser::dims(unsigned long& w, unsigned long& h, unsigned long& d){
    w = (unsigned long)data->pwidth();
    h = (unsigned long)data->pheight();
    d = (unsigned long)data->sectionNo();
}


bool ImageAnalyser::simpleLine(float* line, int xb, int yb, int zb, int l, Dimension dim, unsigned int wi){
    bool ok = false;
    switch(dim){
	case XDIM :   // simple line in the x direction don't bother with the cache
//	    cout << "calling read to float for : XDIM : " << xb << ", " << yb << ", " << zb << endl;
	    ok = data->readToFloat(line, xb, yb, zb, l, 1, 1, wi);
	    break;
	case YDIM :   // first check if we can read it from the cache, if not delete area cache and make a new one
//	    cout << "simpleLine YDIM " << endl;
	    if(!( ok = area_cache->line(line, xb, yb, zb, l, YDIM, wi))){   // this should set the resulting value of ok .. 
		cout << "\t\tneed to make a new cache " << endl;
		float* newCache = new float[area_cache_width * data->pheight()];
		memset((void*)newCache, 0, sizeof(float) * area_cache_width * data->pheight());
		int x = (xb + area_cache_width) > data->pwidth() ? data->pwidth() - area_cache_width : xb;
		if(data->readToFloat(newCache, x, 0, zb, area_cache_width, data->pheight(), 1, wi)){  // delete old cache and make a new one.. 
		    delete area_cache;
		    area_cache = new VolumeCache(newCache, x, 0, zb, area_cache_width, data->pheight(), 1, wi);
		    // and then if we can do this.. 
//		    cout << "\t\t\tnew area cache made and trying to read from it.. " << endl;
		    ok = area_cache->line(line, xb, yb, zb, l, YDIM, wi);
		}else{
		    delete newCache;
		}
	    }    
	    break;
	case ZDIM :
	    if(!(ok = x_z_slice_cache->line(line, xb, yb, zb, l, ZDIM, wi)) ){
		float* newCache = new float[data->sectionNo() * x_z_slice_cache_width * x_z_slice_cache_height];
		memset((void*)newCache, 0, sizeof(float) * data->sectionNo() * x_z_slice_cache_width * x_z_slice_cache_height);
		int x = (xb + x_z_slice_cache_width) > data->pwidth() ? data->pwidth() - x_z_slice_cache_width : xb;
		int y = (yb + x_z_slice_cache_height) > data->pheight() ? data->pheight() - x_z_slice_cache_height : yb;
		cout << "*";
		if(data->readToFloat(newCache, x, y, 0, x_z_slice_cache_width, x_z_slice_cache_height, data->sectionNo(), wi)){
		    delete x_z_slice_cache;
		    x_z_slice_cache = new VolumeCache(newCache, x, y, 0, x_z_slice_cache_width, x_z_slice_cache_height, data->sectionNo(), wi);
		    ok = x_z_slice_cache->line(line, xb, yb, zb, l, ZDIM, wi);
		}else{
		    delete newCache;
//		    cerr << "Unable to obtain z_line for coordinates : " << xb << ", " << yb << ", " << zb << "  length : " << l 
//			 << " range : " << x << " --> " << x + x_z_slice_cache_width << " | " << y << " --> " << y + x_z_slice_cache_height
//			 << "  dims  : " << data->pwidth() << "x" << data->pheight() << endl;
		}
	    }
	    break;
	default :
	    cerr << "ImageAnalyser::SimpleLine unknown dimension : " << dim << endl;
    }
    return(ok);
}

bool ImageAnalyser::point(float& p, int xp, int yp, int zp, unsigned int wi){
    bool ok = false;
    unsigned int vol_cache_width = 256;
    unsigned int vol_cache_height = 256;
    unsigned int vol_cache_depth = data->sectionNo();
    if( !(ok = volume_cache->point(p, xp, yp, zp, wi) )){
	// then we need to work out what cache to get..
      float* newCache = new float[vol_cache_width * vol_cache_height * vol_cache_depth];
      
      //float* newCache = new float[min_point_height * min_point_depth * vol_cache_width];
      // and then work out the xb.. 

      int xb = 256 * (xp / 256);
      int yb = 256 * (yp / 256);
      int zb = 0;
      //	int xb = xp - min_point_height;
      //	xb = (xb + vol_cache_width) >= data->pwidth() ? (data->pwidth() - vol_cache_width) : xb;
      //	xb = xb < 0 ? 0 : xb;
	// the other points are just defined as .. 
	//int yb = yp - (min_point_height / 2);
	//int zb = zp - (min_point_depth / 2);
	// and make sure that the numbers are above 0 and that the numbers + thingy don't overrun the cache
	//yb = (yb + min_point_height) > data->pheight() ? data->pheight() - min_point_height : yb;
	//zb = (zb + min_point_depth) > data->sectionNo() ? data->sectionNo() - min_point_depth : zb;
	//yb = yb < 0 ? 0 : yb;
	//zb = zb < 0 ? 0 : zb;
      if(data->readToFloat(newCache, xb, yb, zb, vol_cache_width, vol_cache_height, vol_cache_depth, wi)){
	delete volume_cache;
	volume_cache = new VolumeCache(newCache, xb, yb, zb, vol_cache_width, vol_cache_height, vol_cache_depth, wi);
	ok = volume_cache->point(p, xp, yp, zp, wi);
      }else{
	delete newCache;
	cerr << "ImageAnalyser::point  Unable to create new cache covering  " << xb << ", " << yb << ", " << zb << "   dims : " 
	     << vol_cache_width << ", " << vol_cache_height << ", " << vol_cache_depth << endl;
      }
    }
    return(ok);
}

threeDPeaks* ImageAnalyser::findAllPeaks(unsigned int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm){   // the same as above but goes through all the slices.. 
    float waveLength = float(data->channel(wl));   // we are using this for to make the simple_drop, but should probably get rid of it
    threeDPeaks* peakInfo = new threeDPeaks(pr, 255, 255, 255);
    
    if(!waveLength){
	cerr << "ImageAnalyser::findAllPeaks unable to find a wavelength for index : " << wl << endl;
	return(peakInfo);
    }
    // find peaks in all three dimensions. Define overlapping positions.
    // Then use the peak information to grow the actual peaks and to define these.
    //
    // basically grow the peak, if appropriate neighbouring peaks are available.
    
    // 1. First define the three different types of peaks..
    map<long, linearPeak> xPeaks;
    map<long, linearPeak> yPeaks;
    map<long, linearPeak> zPeaks;  // this could probably be done faste and neater with sorted vectors, but I've started so..

    
    int lb, bb, rb, tb;  // left and bottom ..
    data->borders(lb, rb, bb, tb);  // take right and top borders as width and height respectively.. 
    lb = bb;  
    // this is a kludge to avoid warning of unused variables.. 
    int w = data->pwidth();
    int h = data->pheight();
    int d = data->sectionNo();

    float* xline = new float[w];
    float* yline = new float[h];
    float* zline = new float[d];


    // First find the x and y Peaks..
    for(int z=0; z < d; ++z){
	cout << "Getting X and Y peaks section : " << z << endl;
	// the xPeaks
	
	for(int y=0; y < tb; ++y){
//	for(int y=0; y < h; ++y){
	    memset(xline, 0, sizeof(float) * w);
	    if(simpleLine(xline, 0, y, z, w, XDIM, wl)){
		vector<linearPeak> peaks = findPeaks(xline, w, pr, minPeakValue, maxEdgeProportion, 0);
		adjustPeaks(peaks, 0, y, z, XDIM);
		for(uint i=0; i < peaks.size(); ++i){
		    xPeaks.insert(make_pair(peaks[i].position, peaks[i]));
		}
	    }
	}
	// thy yPeaks
	for(int x=0; x < rb; ++x){
//	for(int x=0; x < w; ++x){
	    memset(yline, 0, sizeof(float) * h);
	    if(simpleLine(yline, x, 0, z, h, YDIM, wl)){
		vector<linearPeak> peaks = findPeaks(yline, h, pr, minPeakValue, maxEdgeProportion, 1);
		adjustPeaks(peaks, x, 0, z, YDIM);
		for(uint i=0; i < peaks.size(); ++i){
		    yPeaks.insert(make_pair(peaks[i].position, peaks[i]));
		}
	    }
	}
    }
    cout << "Getting z-peaks" << endl;
    // and then the z-peaks..
    // do some profiling to work out where stuff is taking so much time..
    struct timespec time;
    long maxTime, avgTime, minTime;
    for(int y=0; y < tb; ++y){
//    for(int y=0; y < h; ++y){
	cout << y << "   ";
	maxTime = avgTime = 0;
	minTime = MAXLONG;
	int count = 0;
	for(int x=0; x < rb; ++x){
//	for(int x=0; x < w; ++x){
	    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
	    long start = time.tv_nsec;
	    if(simpleLine(zline, x, y, 0, d, ZDIM, wl)){
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
		long end = time.tv_nsec;
		if(end - start > maxTime)
		    maxTime = (end - start);
		if(end - start < minTime)
		    minTime = (end - start);
		avgTime += (end - start);
		++count;
		vector<linearPeak> peaks = findPeaks(zline, d, pr, minPeakValue, maxEdgeProportion, 2);
		adjustPeaks(peaks, x, y, 0, ZDIM);
		for(uint i=0; i < peaks.size(); ++i){
		    zPeaks.insert(make_pair(peaks[i].position, peaks[i]));
		}
	    }
	}
//	printf("%e.2\t%e.2\t%e\n", minTime, maxTime, avgTime / count);
	if(count)
	    cout << (double)minTime << "\t" << (double)maxTime << "\t" << (double)(avgTime / count) << endl;
    }
    cout << endl;
    cout << "Obtained : " << xPeaks.size() << " xPeaks\n"
	 << "         : " << yPeaks.size() << " yPeaks\n"
	 << "         : " << zPeaks.size() << " zPeaks" << endl;
    
    cout << "looking for common peaks " << endl;
    // then all we have to do is find peaks that exist in all 3 dimensions.. and do more stuff with them..
    set<long> commonPeaks;
    
    map<long, linearPeak>::iterator xIt = xPeaks.begin();
    map<long, linearPeak>::iterator yIt = yPeaks.begin();
    map<long, linearPeak>::iterator zIt = zPeaks.begin();
    while(xIt != xPeaks.end() && yIt != yPeaks.end() && zIt != zPeaks.end()){
	// if all are equal to each other, then great..
//	cout << (*xIt).first << "\t" << (*yIt).first << "\t" << (*zIt).first << endl;
	if((*xIt).first == (*yIt).first && (*xIt).first == (*zIt).first){
	    commonPeaks.insert((*xIt).first);
	    xIt++; yIt++; zIt++;
	    continue;
	}
	// at this point we want to increase the iterator with the lowest value..
	if((*xIt).first <= (*yIt).first && (*xIt).first <= (*zIt).first){
	    xIt++;
	    continue;
	}
	if((*yIt).first <= (*xIt).first && (*yIt).first <= (*zIt).first){
	    yIt++;
	    continue;
	}
	if((*zIt).first <= (*xIt).first && (*zIt).first <= (*yIt).first){
	    zIt++;
	}
    }
    cout << "found a load of commone peaks size : " << commonPeaks.size() << endl;
        // at which point we should have a set of common peaks that we can now go through and grow using our already defined linear peaks.
    // so at this point we need some sort of a structure to store the information that we'll be gathering.. 
    
    // in one sense however, we can just use the widths of the three peaks to define a cube containing the droplet, 
    // we could then just take the sum of all the values within the cube, or do some kind of background subtraction or something
    // more complicated to get indicate the signal strength.

    // However, we can still end up with peaks which are next to each other,, esp. if there is something resembling a diagonal line.
    // one way to prevent this is to look within this peak cube area and make sure if there are any other 3D peaks within it.

    // However, it is not clear what should be done if this is the case. One alternative is to keep the highest peak, another alternative is
    // to keep neither peak, as this could indicate something with the wrong shape, or something like a diagonal line of some sort.
    //
    // a third alternative is to consider it as a single peak, and increase the peak dimensions to reflect this. 
    
    // keep the bigger/more intense one... (determine after calculation of the given thing)
    //
    // Note that this still allows some overlap between peaks, as we are only checking for the presence of another peak in the thingy.. 
    
    // let's make a map of simple_drops

    cout << "making drops " << endl;
    map<long, simple_drop> drops;
//    float* source = data + wl * x * y * z;
    for(set<long>::iterator it=commonPeaks.begin(); it != commonPeaks.end(); it++){
	// this means that we can look up the components from xPeaks, yPeaks and zPeaks
	long pos = *it;   // easier to write
	int zp = pos / ( w * h );
	int yp = (pos % (w * h)) / w;
	int xp = (pos % (w * h)) % w;
	// the begin values
	int xb = (xPeaks[pos].begin % (w * h)) % w;
	int xe = (xPeaks[pos].end % (w * h)) % w;
	int yb = (yPeaks[pos].begin % (w * h)) / w;
	int ye = (yPeaks[pos].end % (w * h)) / w;
	int zb = zPeaks[pos].begin / (w * h);
	int ze = zPeaks[pos].end / (w * h);
	
//	cout << "drop at position : " << pos << "  " << xb << "-" << xp << "-" << xe << "  :  " << yb << "-" << yp << "-" << ye << "  :  " << zb << "-" << zp << "-" << ze << endl;

	// now we have a somewhat tricky business. I want to make vector of values. 
 	int diameter = 2 * pr + 1;
	int area = diameter * diameter;
	int volume = diameter * diameter * diameter;

// 	int xdiameter = xe - xb + 1;  // if xe and xb are the same then the width is 1.. not 0
// 	int ydiameter = ye - yb + 1;
// 	int zdiameter = ze - zb + 1;
// 	// we also need offsets as these can be different..
// 	int xo = xp - xb;
// 	int yo = yp - yb;
// 	int zo = zp - zb;

// 	int area = xdiameter * ydiameter;
// 	int volume = area * zdiameter;

	int count = 0;
	vector<float> values(volume, 0);
	for(int dz=(zb - zp); dz <= (ze - zp); dz++){
	    for(int dy=(yb - yp); dy <= (ye - yp); dy++){
		for(int dx=(xb - xp); dx <= (xe - xp); dx++){
		    if(!point(values[ (dz + pr) * area + (dy + pr) * diameter + pr + dx ], xp + dx, yp + dy, dz + zp, wl)){
			cerr << "ImageAnalyser::findAllPeaks unable to assign value to voxel at : " << xp + dx << ", " << yp + dy << ", " << zp + dz << endl;
		    }
		    ++count;
//		    cout << "assigning from : " << (dz + zp) * x * y + (dy + yp) * x + dx + xp << "  --> " << (dz + pr) * area + (dy + pr) * diameter + pr + dx << endl;
//		    values[ (dz + pr) * area + (dy + pr) * diameter + pr + dx ] = source[(dz + zp) * x * y + (dy + yp) * x + dx + xp];  // which shouldn't go out of bounds..
		}
	    }
	}
	cout << count << "  ";
	// and at which point we should assign a new simple_drop..
	drops.insert(make_pair(*it, simple_drop(pr, xp, yp, zp, xb, xe, yb, ye, zb, ze, values, wl, (int)waveLength)));
    }
    cout << endl;
    // and  at this point we might want to clean up drops and make check whether or not we want to play with this. 
    // Note that everything in the loop above can be moved into the loop looking for common peaks, as I don't do anything
    // particularly strange here. But for now it seems simpler to keep the two loops separately, and in the grand scheme of things
    // this isn't likely to affect the speed of the process too much.
    cout << endl << endl << "find all peaks found a total of " << drops.size() << "  peaks " << endl << endl;

    peakInfo->simpleDrops = drops;
    peakInfo->xPeaks = xPeaks;
    peakInfo->yPeaks = yPeaks;
    peakInfo->zPeaks = zPeaks;

    return(peakInfo);

}


threeDPeaks* ImageAnalyser::findAllPeaks_3D(unsigned int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm){
    // get subsets of the data and give it to a threeDPeakFinder objct
    float waveLength = float(data->channel(wl));   // we are using this for to make the simple_drop, but should probably get rid of it
    threeDPeaks* peakInfo = new threeDPeaks(pr, 255, 255, 255);

    cout << "ImageAnalyser::findAllPeaks_3D waveLength is " << waveLength << " an empty peakInfo has been created" << endl;
    
    if(!waveLength){
	cerr << "ImageAnalyser::findAllPeaks unable to find a wavelength for index : " << wl << endl;
	return(peakInfo);
    }

    ThreeDPeakFinder finder;
    
    cout << "ImageAnalyser::findAllPeaks_3D declared a three d peak finder" << endl;

    int w = data->pwidth();
    int h = data->pheight();
    int d = data->sectionNo();
    int sliceHeight = 100 + (2 * pr);   // this is completely arbitrary at the moment, and we'll need to do something smarter about it later on.
    int sliceVolume = d * sliceHeight * w;
    
    map<long, simple_drop> drops;
    float* dataSlice = new float[sliceVolume];
    int xb = 0;
    int zb = 0;
    for(int yb=0; yb < (h - 2 * pr); yb += (sliceHeight - 2 * pr)){
	memset((void*)dataSlice, 0, sizeof(float) * sliceVolume);
	int slice_h = yb + sliceHeight < h ? sliceHeight : h - yb;

	//int y_margin = yb + sliceHeight < h ? pr : 0;  // commenting this out means we can loose some things at the beginning and end, but should be more correct
	int y_margin = pr;
	//cout << "ImageAnalyser::findAllPeaks_3D yb : " << yb << " slice_h : " << slice_h << endl;
	if(!data->readToFloat(dataSlice, xb, yb, zb, w, slice_h, d, wl)){
	    cerr << "ImageAnalyser::findAllPeaks_3D, unable to obtain data for slice from " << xb << ", " << yb << ", " << zb << " ++: " << w << ", " << slice_h << ", " << d << endl;
	    continue;
	}
	//cout << "ImageAnalyser::findAllPeaks_3D calling findaPeaks from finder .. " << endl;
	vector<simple_drop> s_drops = finder.findPeaks(dataSlice, w, slice_h, d, xb, yb, zb, minPeakValue, maxEdgeProportion, pr, (int)waveLength, (int)wl, 0, y_margin, 0);
	//cout << "ImageAnalyser::findAllPeaks_3D and obtained a total of " << s_drops.size() << " peaks" << endl;
	for(uint i=0; i < s_drops.size(); ++i)
	    drops.insert(make_pair(s_drops[i].z * h * w + s_drops[i].y * w + s_drops[i].x, s_drops[i])); // oh dear so ugly..
	//cout << "ImageAnalyser::findAllPeaks_3D inserted these into the stupid long map" << endl;
//	drops.insert(drops.end(), s_drops.begin(), s_drops.end());  // if only I rewrote it not to be soo ugly... 
    }
    //cout << "ImageAnalyser::findAllPeaks_3D obtained a total of " << drops.size() << "  peaks " << endl;
    // and then set thhe relevant values in peakInfo..
    peakInfo->simpleDrops = drops;
    return(peakInfo);
}


void ImageAnalyser::adjustPeaks(vector<linearPeak>& peaks, int bx, int by, int bz, Dimension dim){
    long offset, a, w;
    switch(dim){
	case XDIM :
	    offset = bz * data->pwidth() * data->pheight() + by * data->pwidth();
	    for(uint i=0; i < peaks.size(); ++i){
		peaks[i].offset(offset);
	    }
	    break;
	case YDIM :
	    offset = bz * data->pwidth() * data->pheight() + bx;
	    w = long(data->pwidth());
	    for(uint i=0; i < peaks.size(); ++i){
		peaks[i].setPos(offset + peaks[i].position * w, long(data->pwidth()));
	    }
	    break;
	case ZDIM :
	    a = data->pwidth() * data->pheight();
	    offset = by * data->pwidth() + bx;
	    for(uint i=0; i < peaks.size(); ++i){
		peaks[i].setPos(a * peaks[i].position + offset, long(data->pwidth() * data->pheight()));
	    }
	    break;
	default :
	    cerr << "ImageAnalyser unknown dimension : " << dim << endl;
    }
    
}

vector<linearPeak> ImageAnalyser::findPeaks(float* line, int length, int pr, float minPeakValue, float maxEdgeValue, int dim){
    vector<linearPeak> peaks;
    int i=0;
    float mev;   // mev = peak value multiplied by maxEdgeValue (which is the proportion of the peak value..)
    while(i < length-1){
//	cout << i << ", ";
	int br = 0;          // before radius
	int ps = i;          // peak start
	while(i < length-1 && line[i+1] >= line[i]){
	    br++;
	    i++;
	}
	// peak reached.. if peak satisfies certain criteria then go on and find the next trough..
	if(i >= length-1){           // i.e. we've stopped because we reached the end. (Stopping here means that we can miss an end peak, but I'm going to ignore that for now
	    break;
	}
	// then check for the values..
	// the peak value should be at least minPeakValue
	if(line[i] < minPeakValue){
	    i++;
	    continue;
	}
	int pp = i;
	mev = maxEdgeValue * line[pp];
	
	// if br is larger than pr then check the value at i-pr, otherwise check at i-br..
	if(br > pr && line[i-pr] > mev){
	    i++;
	    continue;
	}
	if(br <= pr && line[i-br] > mev){
	    i++;
	    continue;
	}
	// this peak seems to be fine. Now we have to find the end of the peak.. 
	int ar = 0;   // after radius..
	while(i < length-1 && line[i+1] <= line[i]){
	    i++;
	    ar++;
	}
	if(i >= length-1){   // reached the end of the line.. 
	    break;
	}
	if(ar > pr && line[pp+pr] > mev){
	    continue;
	}
	if(ar <= pr && line[pp+ar] > mev){
	    continue;
	}
	int pe = pr > ar ? pp + ar : pp + pr;
	ps = br > pr ? pp - pr : ps;
	// and if here we actually do have a peak..
	peaks.push_back(linearPeak(pp, ps, pe, dim, pe-ps, line[pp], line[ps], line[pe]));
    }
    return(peaks);
}

vector<Nucleus> ImageAnalyser::findNuclei(float* source, unsigned int w, unsigned int h, float minValue){
    // This is an old and not very smart method of looking for nuclei. I want to replace this at some point using
    // A method that looks for contrasts rather than just minvalue stuff.. -- but that would need some more work
    // and some more development.. -- and some new ways of visualising the stuff.. so that I can judge the consequence
    // of choices made.. oh dear, that sounds a bit troublesome..
    
    unsigned int globalMargin = 40;   // we should have this number set in stone somewhere.. (not sure exactly what I should make it though)
    // we are going to be looking at data only within the globalMargin ignoring data outside of it.
    //
    // perimeters are found using a simple linear scan.. though I'm not sure exactly how to do this yet..
    //
    // First just find a load of horizontal lines and then merge these together to form areas, and from these make perimeters...
    // Sounds pretty simple, but bound to run into difficulties along the lines.. 
    
    // go throuh one line at a time..

    vector<line> lines;
    int minLength = 5;
    for(uint row = globalMargin; row < h - globalMargin; ++row){
	bool inLine = false;
	int start = 0;
	int stop = 0;
	for(uint col = globalMargin; col < w - globalMargin; ++col){
	    if(!inLine){
		if(source[row * w + col] >= minValue){
		    inLine = true;
		    start = row * w + col;
		}
		continue;
	    }
	    // so here must be in the middle of a line.. -- simplest thing to do is 
	    // to just say.. hmm if lower than threshold than stop,, but we should perhaps look forward a little bit.. 
	    // so how about take the average of the next 5 or so values... and if that is more than limit than extend by one
	    // screw that for now, let's just include it if it is somewhere along the other stuff..
	    if(source[row * w + col] < minValue){
		// terminate the line..
		stop = row * w + col;
		if(stop - start >= minLength){
		    lines.push_back(line(start, stop, w));
		}
		start = stop = 0;
		inLine = false;
	    }
	}
	// are we still in a line at the end of the line.. 
	if(inLine){
	    stop = row * w + w - globalMargin - 1;
	    if(stop - start >= minLength){
		lines.push_back(line(start, stop, w));
	    }
	}
    }
    vector<Nucleus> nuclei = findNuclearPerimeters(lines);
    //and do stuff with that..

	    

    return(nuclei);
}

parameterData ImageAnalyser::findContrasts(float* source, unsigned int w, unsigned int h){
    // Define the contrast of a given pixel to be the sum of the absolute differences between
    // it and neighbouring pixels on the positive side .. hmm. 
    // this is not the most obvious way of doing it, but I think it should be ok.. 
    if(!(w * h)){
	cerr << "ImageAnalyser::findContrasts no area specified " << w << ", " << h << endl;
	parameterData d;
	return(d);
    }
    float* contrastData = new float[w * h];
    memset((void*)contrastData, 0, w * h * sizeof(float));

    // and then
    float* s = source;
    float* d = contrastData;
    unsigned int winSize = 5;
    float rx, fx, ry, fy;   // reverse and forward sums..
    float mx, my;           // x and y magnitude.. 
    for(uint y=winSize; y < (h - (winSize + 1)); ++y){
//	s = source + y * w;
//	d = contrastData + y * w;
	for(uint x=winSize; x < (h -(winSize + 1)); ++x){
	    rx = fx = ry = fy = 0;
	    for(uint i=1; i <= winSize; ++i){
		rx += s[y * w + x] - s[y * w + x - i]; 
		fx += s[y * w + x + i] - s[y * w + x];

		ry += s[y * w + x] - s[(y - i) * w + x];
		fy += s[(y + i) * w + x] - s[y * w + x];

	    }
	    mx = rx * fx;
	    my = ry * fy;    // hence if both are the same sign we get a positive number, otherwise we get a negative number.. 

	    if(mx + my > 0){
		d[y * w + x] = mx + my;
	    }
//	    *d += fabsf(s[0] - s[1]);
//	    *d += fabsf(s[0] - s[w]);
//	    ++d;
//	    ++s;
	}
    }
    // and make a parameterData set .. whoallal..
    return(parameterData(contrastData, w, h));
}

vector<Nucleus> ImageAnalyser::findNuclearPerimeters(vector<line>& lines){
    // look at the nuclear lines and make perimeters from these that can be drawn.. (might want to 
    // integrate the signal strength over the nuclei as well.. 
    
    // sort the lines (though I'm not sure if we really need to)
    sort(lines.begin(), lines.end());

    set<line> mergedLines;    // for storing lines which have already been merged.. 
    set<set<line> > nuclearSets;  // sets of lines representing nuclei..

    for(uint i=0; i < lines.size(); ++i){
	if(mergedLines.count(lines[i])){
	    continue;
	}
	set<line> currentLines;
	lines[i].neighboringLines(lines, currentLines);

	mergedLines.insert(currentLines.begin(), currentLines.end());
	nuclearSets.insert(currentLines);
	currentLines.erase(currentLines.begin(), currentLines.end());
    }
    cout << "ImageAnalyser::findNuclearPerimeters obtained a total of " << nuclearSets.size() << "  sets of nuclear lines" << endl;
    
    vector<vector<twoDPoint> > perimeters;
    vector<Nucleus> nuclei;
    for(set<set<line> >::iterator it = nuclearSets.begin(); it != nuclearSets.end(); it++){
	Nucleus nucleus = findNuclearPerimeter((*it));
	nuclei.push_back(findNuclearPerimeter((*it)));
    }
    return(nuclei);
}

Nucleus ImageAnalyser::findNuclearPerimeter(const set<line>& lines){
    Nucleus nucleus;
    set<line>::iterator it;
    vector<twoDPoint> points;
    int minCol, maxCol;
    int minRow, maxRow;
    it = lines.begin();
    if(it == lines.end()){
	cerr << "no lines in line set " << endl;
	return(nucleus);
	//return(points);
    }
    minCol = (*it).col_start;
    maxCol = (*it).col_stop;
    minRow = maxRow = (*it).rn;
    for(it=lines.begin(); it != lines.end(); it++){
	if(minCol > (*it).col_start){ minCol = (*it).col_start; }
	if(maxCol < (*it).col_stop){ maxCol = (*it).col_stop; }
	if(minRow > (*it).rn){ minRow = (*it).rn; }
	if(maxRow < (*it).rn){ maxRow = (*it).rn; }
    }
    nucleus.min_x = minCol;
    nucleus.max_x = maxCol;
    nucleus.min_y = minRow;
    nucleus.max_y = maxRow;
    nucleus.lines = lines;
    
    cout << "minCol : " << minCol << "  maxCol : " << maxCol << "  minRow : " << minRow << "  maxRow : " << maxRow << endl;
    int nuc_width = 2 + 1 + maxCol - minCol;
    int nuc_height = 2+ 1 + maxRow - minRow;    // the two plus is to make sure that we have 
    // and make a vector of booleans that fills this space..
    vector<bool> nuc_mask(nuc_width * nuc_height, false);
    // and then fill this boolean with stuff..
    for(it = lines.begin(); it != lines.end(); it++){
	int yp = 1 + (*it).rn - minRow;    // the yposition.. translated into the local coordinates .. (with a margin of 1 position) 
	for(int x_p = (*it).col_start; x_p <= (*it).col_stop; x_p++){
	    int xp = 1 + x_p - minCol;    // local coordinates with a one thingy.. 
	    nuc_mask[yp * nuc_width + xp] = true;
	}
    }
    // and now we have a mask we can go around the mask and fill in stuff..
    int xoffsets[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int yoffsets[] = {0, -1, -1, -1, 0, 1, 1, 1};     // this gives us a anticlockwise spin around a central position.. 
    int offset_offsets[] = {6, 6, 0, 0, 2, 2, 4, 4};  // how the offset position should move as a result of stuff.. 

    //    7 6 5                                 X 6 5
    //    0 X 4   new boundary pos at 3  -->    0 Y 4   then 0 is the offset of the non thingy position.. (hmm). 
    //    1 2 3                                 1 2 3

    
    // basically we will find a starting point, then from this point we'll follow the offsets around until 
    // we hit the pattern, false followed by true. the true position is our next point in the thingy.. 
    
    // First we have to find a starting position..
    int start_pos = 0;
    for(uint i=0; i < nuc_mask.size(); i++){
	if(nuc_mask[i]){
	    start_pos = i;
	    break;
	}
    }
    
    points.push_back(twoDPoint(minCol - 1 + (start_pos % nuc_width), minRow - 1 + (start_pos / nuc_width)));
    // and then use the offsets to find the next one..
    int col_pos = start_pos % nuc_width;
    int row_pos = start_pos / nuc_width;
    int offsetPos = 0;     // which is the position on the left of the current position.. 
    // then try to find the next position..
    bool keep_going = true;
    while(keep_going){
	// go through the 8 positions..	
	for(int i=0; i < 8; i++){
	    int op = (offsetPos + i) % 8;    // somewhere between 0 and 7 .. 
	    // because the original position has to be false we can just say if we come across
	    // a true position that is our next member.. 
	    if(nuc_mask[ (row_pos + yoffsets[op]) * nuc_width + (col_pos + xoffsets[op])]){
		// then this position is our new boundary thingy.. 
		// and the op - 1 is our new offsetPos .. 
		// first check if we've gone all the way around..
		if((row_pos + yoffsets[op]) * nuc_width + (col_pos + xoffsets[op]) == start_pos){
		    keep_going = false;
		    break;
		}
		// otherwise push back the points and set the offsets and stuff appropriately..
		points.push_back(twoDPoint(minCol - 1 + col_pos + xoffsets[op], minRow - 1 + row_pos + yoffsets[op]));
		row_pos = row_pos + yoffsets[op];
		col_pos = col_pos + xoffsets[op];
//		cout << "  " << col_pos << "," << row_pos;
		// and finally the important thing..
		offsetPos = offset_offsets[op];    // which should be ok.. 
		break;
	    }
	}
    }
//    cout << endl;
    nucleus.perimeter = points;
//    nucleus.smoothenPerimeter();  // turns out not to be so useful.. 
    return(nucleus);
    
}

parameterData ImageAnalyser::findSets(float* source, unsigned int w, unsigned int h, int minSize, int maxSize, float minValue){
    // We need
    // 1. float* values for parameter data. Make the values of this be simply the number of sets a given pixel belongs to
    // 2. char* mask to make sure we don't end up going in circles wasting our time..
    // 3. vector<int> members..
    char* mask = new char[w * h];
    float* values = new float[w * h];
    memset((void*)values, 0, w * h * sizeof(float));
    
    vector<int> members;
    members.reserve(maxSize+1);  // one bigger in case I'm being stupid somewhere.
    int setSize;
    float setSum;
    
    cout << "ImageAnalyser::findSets called with minSize : " << minSize << "  maxSize : " << maxSize << "  and minValue : " << minValue << endl;

    int setCounter = 0;
    // then simply go through all of the pixels and if above the minValue make a set..
    for(unsigned int y=0; y < h; ++y){
	cout << "y : " << y << endl;
	for(unsigned int x=0; x < w; ++x){
	    if(source[y * w + x] >= minValue){
		memset((void*)mask, 0, w * h * sizeof(char));
		members.resize(0);
		setSize = 0;
		setSum = 0;
		expandSet(source, mask, w, h, x, y, setSize, setSum, maxSize, members, source[y * w + x]);  // which is recursive. and does everything..
		cout << "_";
		// now check on the values and if they are ok, then increase the values of the thingy in a reasonable manner... 
		if(setSize < maxSize && setSize > minSize){     // ok, do something useful
		    cout << "\nObtained a set starting at coordinate : " << x << ", " << y << " sum : " << setSum << "  set No: " << setCounter++ << "  size : " << setSize 
			 <<  endl;
		    for(uint i=0; i < members.size(); ++i){
			values[members[i]]++;
		    }
		}
		/// This isn't very useful at the moment, but later on I can also make a circumference of the area.. and then maybe do something smarter.. 
	    }
	}
    }
    delete mask;
    // make a parameter Data and return that.. 
    return(parameterData(values, w, h));
}

void ImageAnalyser::expandSet(float* source, char* mask, unsigned int w, unsigned int h, unsigned int x, unsigned int y, int& setSize, float& setSum, int maxSize, std::vector<int>& members, float minValue){
    // if neighbouring point has a higher value than this point then add it to the set (set the mask)
    // and call expandSet on that point.

    /// Actually this has a BUG -- it should take a specific value.. 

    if(x >= w || y >= h){
	return;
    }
    int p = y * w + x;
    mask[p] = 1;
    members.push_back(p);

    // make sure that size is not too large.. 
    ++setSize;
    setSum += source[p];
    
    cout << ".";

    if(setSize > maxSize){
	return;
    }
    // check to the left ..
    if(x > 0 &&
       !mask[p - 1] &&
       minValue <= source[p - 1]){
//       source[p] <= source[p - 1]){
	expandSet(source, mask, w, h, x-1, y, setSize, setSum, maxSize, members, minValue);
    }
    // check to the top
    if(y <= h - 1 &&
       !mask[p + w] &&
       minValue <= source[p + w]){
//       source[p] <= source[p + w]){
	expandSet(source, mask, w, h, x, y+1, setSize, setSum, maxSize, members, minValue);
    }
    // check to the right
    if(x <= w - 1 &&
       !mask[p + 1] &&
       minValue <= source[p + 1]){
//       source[p] <= source[p + 1]){
	expandSet(source, mask, w, h, x+1, y, setSize, setSum, maxSize, members, minValue);
    }
    // check to the bottom
    if(y > 0 &&
       !mask[p - w] &&
       minValue <= source[p - w]){
//       source[p] <= source[p - w]){
	expandSet(source, mask, w, h, x, y-1, setSize, setSum, maxSize, members, minValue);
    }
    // and that's it folks..
}

PerimeterData ImageAnalyser::findPerimeters(float* source, unsigned int w, unsigned int h, int minSize, int maxSize, float minValue){
    // finds overlapping perimeters (well kind of anyway..)

    // Find a point with a value above minValue
    // From that point find a perimeter where all points satisfy the following criteria :
    //     a. are higher than the starting point
    //     b. are not members of any other perimeters (use a global mask to make sure it is ok)
    // 
    //  To follow the perimeter use the same logic as in the findNuclearPerimeter thingy.. 

    // first make a binary mask
    char* mask = new char[w * h];
    float* values = new float[w * h];
    
    int minArea = minSize * minSize;

    // zero both
    memset((void*)mask, 0, w * h * sizeof(char));
    memset((void*)values, 0, w * h * sizeof(float));

    // then go through the source points and if we find a point with a value higher than minValue then try to expand the perimeter..
    int pos;

    vector<PerimeterSet> perimeterSets;

    int perimeterNo = 0;
    for(uint y=0; y < h; ++y){
	for(uint x=0; x < w; ++x){
	    pos = y * w + x;
//	    cout << "::" << x << "," << y << "::";
	    if(!mask[pos] && source[pos] >= minValue){
		int minX, minY, perWidth, perHeight;
		// the next call passes stuff by references and sets the values of things like minX, minY, perWidth etc.. 
		vector<int> perimeter = expandPerimeter(source, mask, w, h, source[pos], pos, maxSize, minX, minY, perWidth, perHeight);
		// and at this point we need to do something with the perimeter.. 
		if(perimeter.size() <= maxSize && perimeter.size() >= minSize && perWidth * perHeight > minArea){  // we call it a valid perimeter..
		    // do something smart .. like..
		    perimeterNo++;
//		    cout << "new perimeter found from " << x << "," << y << "\torigin : " << minX << "," << minY << " size: " << perWidth << "x" << perHeight << endl;
		    for(uint i=0; i < perimeter.size(); ++i){
			mask[perimeter[i]] = 1;
			int px = perimeter[i] % w;
			int py = perimeter[i] / w;
			int plx, ply;
			if(i){
			    plx = perimeter[i - 1] % w;
			    ply = perimeter[i - 1] / w;
			}else{
			    plx = perimeter.back() % w;
			    ply = perimeter.back() / w;
			}
			if(px - plx && py - ply){   // if we moved along a diagonal we should make a line that can't be crossed .. 
			    mask[py * w + plx] = 1;
//			    mask[ply * w + px] = 1;  // only one of these should be necessary.. and which one shouldn't really matter.. 
			}
			
		    }
		    fillPerimeter(values, w, h, perimeter, minX, minY, perWidth, perHeight);

		    Perimeter per(perimeter, w, h, minX, minY, minX + perWidth, minY + perHeight);
		    bool addedToSet = false;
		    for(uint i=0; i < perimeterSets.size(); ++i){
//			cout << "Calling addPerimeter on set : " << i << endl;
			if(perimeterSets[i].addPerimeter(per, source)){
			    addedToSet = true;
			    break;
			}
		    }
		    if(!addedToSet){
			cout << "\n\n PERIMETER NOT OVERLAPPED WITH ANY SETS MAKING NEW SET FOR PERIMETER: " << endl;
			per.printRange();
			cout << "\n\n" << endl;
			PerimeterSet ps;
			ps.addPerimeter(per, source);
			perimeterSets.push_back(ps);
		    }
		}else{
		    // we might want to reset the mask if we don't consider this one to be valid as it might form some other valid lines.. hmm.

		}
	    }
	}
    }
    cout << "just about to delete mask in imageAnalyser " << endl;
    // and at this point make a perimeter data thingy.. and do something smart with that.. 
    delete mask;
    cout << "findPerimeters found a total of " << perimeterNo << " perimeters" << endl;
    cout << "                       total of " << perimeterSets.size() << " sets" << endl;
    PerimeterData perd(parameterData(values, w, h), perimeterSets);
    return(perd);
//    return(parameterData(values, w, h));
}

// returns an array of floats of pixels within the given perimeter
float* ImageAnalyser::perimeterPixels(Perimeter per, int z, unsigned int wi, unsigned int& p_length, float& pixel_sum)
{
  int x = per.xmin();
  int y = per.ymin();
  int w = 1 + per.xmax() - x;
  int h = 1 + per.ymax() - y;

  if(w < 1 || h < 1)
    return(0);
  
  float* img_data = new float[ w * h ];
  memset((void*)img_data, 0, sizeof(float) * w * h);
  if(!data->readToFloat(img_data, x, y, z, w, h, 1, wi, true)){ // use_cmap=true
    delete []img_data;
    return(0);
  }
  
  char border_value = 1;
  char out_value = 2;
  int mw, mh, m_xo, m_yo;  // the mask coordinates and borders
  char* perMask = per.makeMask(border_value, out_value, mw, mh, m_xo, m_yo);
  // this mask has 0 values for positions within the boundary and non-zero values
  // for positions outside of the boundary. This is to allow encoding of overlapping
  // borders and such niceties.
  if(!perMask || m_xo > x || m_yo > y){
    std::cerr << "ImageAnalyser::perimeterPixels makeMask returned 0 or other bad coordinate" << std::endl;
    delete []img_data;
    return(0);
  }
  float* pixel_values = new float[ w * h ];
  pixel_sum = 0;
  p_length = 0;
  
  int my = m_yo - (y + 1);
  for(int yp=y; yp < (y + h); ++yp){
    ++my;
    int mx = m_xo - (x + 1);
    for(int xp=x; xp < (x + w); ++xp){
      ++mx;
      std::cout << yp << "," << xp << " : " << p_length << "  : " << img_data[ yp * w + xp ] << " --> " << pixel_sum << std::endl;
      if(!perMask[ my * mw + mx ]){
	pixel_values[p_length] = img_data[ yp * w + xp ];
	pixel_sum += pixel_values[p_length];
	++p_length;
      }
    }
  }
  delete []img_data;
  return(pixel_values);
}

// This looks like a BAD FUNCTION that should be removed ASAP.
// Unfortunately it's used by findPerimeters, to fill up the
// perimeterData structure. It looks very likely that there is
// a memory leak somewhere.
void ImageAnalyser::fillPerimeter(float* dest, unsigned int w, unsigned int h, vector<int>& perimeter, int minX, int minY, int perWidth, int perHeight){
    cout << "ImageAnalyser fillPerimeter called " << endl;
    // in order to do this, make a mask, and define the points within that. 
    char* perMask = new char[perWidth  * perHeight];  
    memset((void*)perMask, 0, perWidth * perHeight  * sizeof(char));

    cout << "fillPerimeter length " << perimeter.size() << " x_dom : " << minX << " --> " << minX + perWidth << "\ty_dom : " << minY << " --> " << minY + perHeight << endl;
    vector<int> perCount(perimeter.size(), 0);
    char bvalue = 1;
    for(uint i=0; i < perimeter.size(); ++i){
	int x = perimeter[i] % w;
	int y = perimeter[i] / w;

	int px = x - minX;
	int py = y - minY;

//	dest[(py + minY) * w + px + minX]++;
	perMask[py * perWidth + px] = bvalue;
    }

    char outValue = 2;
    // finding a point that is definetely inside the perimeter is difficult, however, finding a point that is definetely outside the perimeter is much 
    // easier.. (since we can guarantee that a point that has never touched a wall will be thingy..
    for(int py = 0; py < perHeight; ++py){
	if(!perMask[py * perWidth]){
//	    cout << "calling flood fill 0," << py << endl;
	    floodFill(perMask, perWidth, perHeight, bvalue, outValue, 0, py);
	}
	if(!perMask[py * perWidth + perWidth - 1]){
//	    cout << "calling flood fill " << perWidth-1 << "," << py << endl;
	    floodFill(perMask, perWidth, perHeight, bvalue, outValue, perWidth-1, py);
	}
    }
    cout << "finished flood filling from the left and right boundaries" << endl;
    for(int px = 0; px < perWidth; ++px){
	if(!perMask[px]){
//	    cout << "calling flood fill " << px << ",0" << endl;
	    floodFill(perMask, perWidth, perHeight, bvalue, outValue, px, 0);
	}
	if(!perMask[(perHeight - 1) * perWidth + px]){
//	    cout << "calling flood fill " << px << "," << perHeight - 1 << endl;
	    floodFill(perMask, perWidth, perHeight, bvalue, outValue, px, perHeight - 1);
	}
    }
    // so if that is correct then only false values are defined as inside the perimeter.. (not part of the perimeter as well).
    
    cout << "flood fill returned, now we can do other stuff.." << endl;
    // then go through the mask and determine if we are inside or outside of the thingy
    // Note that this algorithm has two problems
    // 1. left boundaries get included but not right boundarids
    // 2. horizontal lines will be half included.. 
    for(int py=0; py < perHeight; ++py){
	for(int px=0; px < perWidth; ++px){
	    if(!perMask[py * perWidth + px]){
		dest[(py + minY) * w + px + minX]++;
	    }
	}
    }

    delete perMask;
}

void ImageAnalyser::floodFill(char* mask, int perWidth, int perHeight, char bvalue, char fvalue, int x, int y){
    // the mask contains all os except for a perimeter point.. 
    // start at intenal position x, y (and fill values with fvalue)
  if(x == 80 && y == 354)
    cout << "hello : " << x << "," << y << endl;
  int p = y * perWidth + x;
  //cout << "\tbeginning of flood fill p: " << p << " : " << x << "," << y << " pwidth : " << perWidth << " pheight : " << perHeight << endl;
  mask[p] = fvalue;
  //cout << "\t\t" << p << " : " << perWidth << ", " << perHeight << endl;
  // check values in all four directions..
  
  if(x - 1 >= 0 && !mask[p - 1]){
    floodFill(mask, perWidth, perHeight, bvalue, fvalue, x-1, y);
  }
  if(x + 1 < perWidth && !mask[p + 1]){
    floodFill(mask, perWidth, perHeight, bvalue, fvalue, x+1, y);
  }
  if(y - 1 >= 0 && !mask[p - perWidth]){
    floodFill(mask, perWidth, perHeight, bvalue, fvalue, x, y-1);
  }
  if(y + 1 < perHeight && !mask[p + perWidth]){
    floodFill(mask, perWidth, perHeight, bvalue, fvalue, x, y+1);
  }
  //    cout << "end of flood fill" << endl;
}


vector<int> ImageAnalyser::expandPerimeter(float* source, char* mask, unsigned int w, unsigned int h, float minValue, int origin, int maxSize, int& minX, int& minY, int& perWidth, int& perHeight){
    
    // CAUTION CAUTION
    // I'm not sure whether it is a good idea to include the mask check, but let's do so for now
    //   cout << "\texpandPerimeter" << endl;
    // the below arrays describe the order in which neighbouring pixels are checked
//    int xoffsets[] = {-1, -1, 0, 1, 1, 1, 0, -1};
//    int yoffsets[] = {0, -1, -1, -1, 0, 1, 1, 1};     // this gives us a anticlockwise spin around a central position.. 
//    int offset_offsets[] = {6, 6, 0, 0, 2, 2, 4, 4};  // how the offset position should move as a result of stuff.. 

    // I think that the below makes more sense (less optimised though)
//    int xoffsets[] = {-1, -1, 0, 1, 1, 1, 0, -1};
//    int yoffsets[] = {0, -1, -1, -1, 0, 1, 1, 1};     // this gives us a anticlockwise spin around a central position.. 
//    int offset_offsets[] = {6, 6, 0, 0, 2, 2, 4, 4};  // how the offset position should move as a result of stuff.. 

    int xoffsets[] = {-1, 0, 1, 1, 1, 0, -1, -1};
    int yoffsets[] = {1, 1, 1, 0, -1, -1, -1, 0};     // this gives us a clockwise spin around a central position.. 
    int offset_offsets[] = {5, 6, 7, 0, 1, 2, 3, 4};  // how the offset position should move as a result of stuff.. 

    //    0 1 2                                      X 4 5
    //    7 X 3   new boundary pos (Y)  at 0  -->    2 Y 6   then 0 is the offset of the non thingy position.. (hmm). 
    //    6 5 4                                      1 0 7

    vector<int> perimeter;
    perimeter.reserve(maxSize + 1);
    perimeter.push_back(origin);
    // origin is the starting point for filling in the perimeter
    int y = origin / w;
    int x = origin % w;
    int offsetPos = 0;   // the position in the offset_offsets vector.. 
    minX = x;
    minY = y;

    // we also need a couple of points for the top right corner of the perimeter.. 
    int maxX = minX;
    int maxY = minY;
    
    //   cout << "origin : " << x << ", " << y << endl;
    bool keep_going = true;
    while(keep_going){
	bool pointAdded = false;
	for(int i=0; i < 8; i++){
	    int op = (offsetPos + i) % 8;
	    // make sure that the new position is within bounds..
	    int nx = x + xoffsets[op];
	    int ny = y + yoffsets[op];
	    int np = ny * w + nx;
	    //   cout << "op : " << op << " : " << nx << ", " << ny << endl;
	    if(np == origin){
//		cout << "\nnp == origin" << endl;
		keep_going = false;
		perimeter.push_back(np); // which should not be necessary ? 
		pointAdded = true; // white lie.. 
		break;
	    }
	    if(nx < 0 || nx >= w || ny < 0 || ny >= h){
//		cout << "|";
		continue;
	    }
	    // then check if this position satisfies the criteria..
//	    if(source[np] >= minValue){
	    
	    float maxProp = 2.0;
//	    if(source[np] >= minValue && source[np] <= (minValue * maxProp)){
	    if(!mask[np] && source[np] >= minValue){
		// add this point to the chain
//		cout << "\nadding point : " << nx << ", " << ny << endl;
		perimeter.push_back(np);
//		mask[np] = 1;
		// check if we need to update mins and maxes..
		if(nx > maxX){ maxX = nx;}
		if(nx < minX){ minX = nx;}
		if(ny > maxY){ maxY = ny;}
		if(ny < minY){ minY = ny;}
		// and set up the next offsetPos.
		offsetPos = offset_offsets[op];
		x = nx;
		y = ny;
		pointAdded = true;
		break;
	    }
	}
	// if we get here and the length of perimeter is 1, then we
	// were unable to add any points to it. so we don't want to continue..
//	cout << "_" << perimeter.size() << "_";
	if(perimeter.size() == 1 || perimeter.size() > maxSize){ 
	    keep_going = false;
	}
//	if(!pointAdded){
//	    exit(1);
//	}
    }
    perWidth = 1 + maxX - minX;
    perHeight = 1 + maxY - minY;
//    cout << "perimeter size : " << perimeter.size() << "  width : " << perWidth << "  height " << perHeight << endl;
    return(perimeter);
}
