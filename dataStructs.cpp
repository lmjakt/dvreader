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

#include "dataStructs.h"
#include "stat/stat.h"
#include <algorithm>
#include <iostream>
#include <math.h>

using namespace std;

distribution::distribution(vector<float> values, uint bno){
    if(!values.size() || !bno){
	minValue = maxValue = 0;
	return;
    }
    // find min and max values..
    minValue = maxValue = values[0];
    for(uint i=0; i < values.size(); i++){
	if(values[i] > maxValue){ maxValue = values[i]; }
	if(values[i] < minValue){ minValue = values[i]; }
    }
    if(maxValue == minValue){
	// we'll get divide by 0's..
	cerr << "distribution constrctor, min and max values are the same, adding a pseudocount to maxValue" << endl;
	maxValue = maxValue * 1.01;
    }
    // initialise the values..
    counts.resize(bno);
    for(uint i=0; i < counts.size(); i++){
	counts[i] = 0;
    }
    // and then simply calculate the distribution.. 
    float range = maxValue - minValue;
    for(uint i=0; i < values.size(); i++){
	int div = int( float(bno - 1) *  (values[i] - minValue) / range );
	counts[div] += (1.0 / float(values.size()));   // this may not be the fastest way of doing things, but.. there you go..
    }
}

void distribution::printDistribution(){
    if(!counts.size()){
	return;
    }
    float range = maxValue - minValue;
    float div = range / float(counts.size());
    for(uint i=0; i < counts.size(); i++){
	float v = minValue + i * div;
	cout << i << "\t" << v << "\t" << counts[i] << endl;
    }
}

// this is a complete copy of the dropVolume function. (it is also 
// implemented in Cluster since I was trying to keep the simple_drop
// struct much simpler. Still, this seems to be the best choice
vector<float> simple_drop::centralLine(uint dim){    
    int s = 1 + radius * 2;
    int S = s * s;
    vector<float> line(s, 0);
    if(dim > 2){
	cerr << "simple_drop centralLine dimension too high : " << endl;
	return(line);
    }
    if(values.size() != S * s){
	cerr << "simple_drop centralLine values size is not : " << s * S << endl;
	return(line);
    }
    int xp, yp, zp;   // the coordinates..
    xp = yp = zp = radius;  // the center position.
    for(int i=0; i < s; i++){
	switch(dim){
	    case 0 :
		xp = i;
		break;
	    case 1 :
		yp = i;
		break;
	    default :
		zp = i;
	}
	// then line[i] is just whatever happens to lie at the appropriate position..
	line[i] = values[zp * S + yp * s + xp];
    }
    return(line);
}

void dropVolume::setShapes(){
    // this is kind of like looking at the distribution in the three different 
    // dimensions.. which is somewhat akin to getting the shape of the dot
    // assuming that it is not complex..
    int s = 2 * radius + 1;   // the size in each dimension.
    int S = s*s;

    // All that doesn't seem to do very much useful stuff, so here I'm going to go through the values and calculate some different stuffss
    // and see what we can do with that..
    background = 0;

    for(int i=0; i < S; i++){
	int p = s * i;
	float mv = values[p];
//	cout << i;
	for(int j=0; j < s; j++){
	    if(values[p + j] < mv){ mv = values[p + j]; }
	    //cout << "\t" << p+j << ":" << values[p+j];
	}
//	cout << "\t: " << mv << endl;
	background += mv;
    }
    background = background / float(S);   // which should be fine.. 
//    cout << "background calculated : " << background << endl;


    x_shape.resize(s);
    y_shape.resize(s);
    z_shape.resize(s);

    // and do the same for the other kinds of things..
    x_peak_count.resize(s);
    y_peak_count.resize(s);
    z_peak_count.resize(s);
    x_peak_shape.resize(s);
    y_peak_shape.resize(s);
    z_peak_shape.resize(s);

    for(int i=0; i < s; i++){
	x_shape[i] = 0;
	y_shape[i] = 0;
	z_shape[i] = 0;
	x_peak_count[i] = 0;
	y_peak_count[i] = 0;
	z_peak_count[i] = 0;
	x_peak_shape[i] = 0;
	y_peak_shape[i] = 0;
	z_peak_shape[i] = 0;
    }
    // since we are going to do this in linear terms for each shape
    // we are going to do a whole load of counting..
    vector<float> xv(s);
    vector<float> yv(s);
    vector<float> zv(s);

    vector<float> column_values;
    vector<float> frame_values;    // 
    column_values.reserve(s * s * s);
    frame_values.reserve(s * s * s);

    for(int i=0; i < s; i++){
	for(int j=0; j < s; j++){
	    for(int k=0; k < s; k++){
		int xp = i*S + j*s + k;
		int yp = i*S + k*s + j;
		int zp = k*S + j*s + i;
		x_shape[k] += values[xp];
		y_shape[k] += values[yp];
		z_shape[k] += values[zp];
		
		xv[k] = values[xp];
		yv[k] = values[yp];
		zv[k] = values[zp];

		column_values.push_back(yv[k]);
		frame_values.push_back(zv[k]);
		// and then use these to set the peak shapes.. (which is more of a thingy.. )
	    }
	    addPeakValues(xv, x_peak_count, x_peak_shape);
	    addPeakValues(yv, y_peak_count, y_peak_shape);
	    addPeakValues(zv, z_peak_count, z_peak_shape);  // which should be enough.. 
	}
    }
    // which should be fine, the only problem being that I now need to report them somehow. (maybe I can just print out the numbers to begin with.. 
    // if the drop is symmetrical it could be considered as a sum of the above thingies..
    
    addPeakDistances(values, x_peak_dist);
    addPeakDistances(column_values, y_peak_dist);
    addPeakDistances(frame_values, z_peak_dist);

    // and then... 
//    meanValue = mean(values);
//    std = std_dev(values);          // which I know is a bit ugly, but I'm a bit tired here... 
	
    kernelValues.resize(values.size());
    for(uint i=0; i < values.size(); i++){
	kernelValues[i] = 0;
    }
    
    // Starting from the core position in the drop, work backwards and fill in the appropriate bits... 
    // not sure of the best algorithm, but let's have a go.. 
    
    // first in reverse..
    int cp = S * radius + s * radius + radius;  // no need to add 1 since we start counting from 0 .. 
    float minSignal = backgroundMultiplier;       // signal should be 4 times the background..
    float minValue = background * minSignal;
    kernelValues[cp] = values[cp];   // though we should check that the kernel Value is high enough..
//    if(values[cp] < background * minSignal){
//	cerr << "Core kernel value is too small value : " << values[cp] << "\tbackground : " << background << endl;
//    }
    // but still leave it there.. 
    //
    // we need to call growKernel with all neighbouring voxels.. 
    for(int dx=-1; dx <= 1; dx++){
	for(int dy=-1; dy <= 1; dy++){
	    for(int dz=-1; dz <= 1; dz++){
		if(dx || dy || dz){     // no point to do with 0, 0, 0...
		    //cout << "calling growKernel " << radius << ", " << radius << ", " << radius << "  with " << dx << ", " << dy << ", " << dz << endl;
		    growKernel(radius, radius, radius, dx, dy, dz, minValue);   // which is a recursive function.. 
		}
	    }
	}
    } 
    totalValue = 0;
    int kCount = 0;
    for(uint i=0; i < kernelValues.size(); i++){
	totalValue += kernelValues[i];
	if(kernelValues[i] > 0){
	    kCount++;
	}
    }
    //cout << "Kernel uses " << kCount << " out of " << kernelValues.size() << endl;

    // but I don't think that will help very much.. still,, we'll see .. 

//     // and then set the full gaussian values. This is ugly, and takes for ever.. 
//     gaussian_values.resize(values.size());
//     for(uint i=0; i < values.size(); i++){
// 	gaussian_values[i] = 0.0;
//     }
//     // and then .. the long, long way of doing it..
//     for(int z1=0; z1 < s; z1++){
// 	for(int y1=0; y1 < s; y1++){
// 	    for(int x1=0; x1 < s; x1++){
// 		// and then just repeat.. but with z2, y2, and x2
// 		for(int z2=0; z2 < s; z2++){
// 		    for(int y2=0; y2 < s; y2++){
// 			for(int x2=0; x2 < s; x2++){
// 			    // and then we can easily work out the positions as..
// 			    int p1 = z1 * S + y1 * s + x1;
// 			    int p2 = z2 * S + y2 * s + x2;
// 			    float d = float((z1 - z2) * (z1 - z2) + (y1 - y2) * (y1 - y2) + (x1 - x2) * (x1 - x2));
// 			    gaussian_values[p1] += values[p2] * expf(-d);   // hey, a three dimensional gaussian blur.. 
// 			}
// 		    }
// 		}
// 	    }
// 	}
//     }
//     // which seems very troublesome indeed... 


// and then a completely new thing again. Calculate variances for the guassian fucntion from the central lines only..
    vector<float> xc = centralLine(0);
    vector<float> yc = centralLine(1);
    vector<float> zc = centralLine(2);
    float mean;  //
    float fd = float(xc.size());
    dist_stats(mean, xVariance, xc, 0, fd);
    dist_stats(mean, yVariance, yc, 0, fd);
    dist_stats(mean, zVariance, zc, 0, fd);

    // which is all I need to do here.. for now.. 
}

void dropVolume::addPeakValues(vector<float>& v, vector<float>& c_shape, vector<float>& v_shape){     // inline since it has some other issues.. 
    // special case if we start with a peak we have only one position..
    if(v[0] > v[1]){
	c_shape[0] += 1.0;
	// now I'm thinking that it would be nice to subtract background levels (i.e. throughs, but, not sure if it is so easy)
	// so I won't bother for now..
	v_shape[0] += v[0];
    }
    for(uint i=1; i < v.size() - 1; i++){
	if(v[i] > v[i-1] && v[i] > v[i+1]){   // the minimal definition of a peak
	    c_shape[i] += 1.0;
	    v_shape[i] += v[i];
	}
    }
    // and the last position is a special case.. again..
    if(v.back() > v[v.size() - 2]){
	c_shape[v.size() - 1] += 1.0;
	v_shape[v.size() - 1] += v.back();
    }
}

void dropVolume::addPeakDistances(vector<float>& v, vector<float>& dist){
    // distance may be larger than s, but should be smaller than 2*s. // for now let's assume no more than 2 * s.
    int s = radius * 2 + 1;
    dist.resize(2 * s);
    for(uint i=0; i < dist.size(); i++){
	dist[i] = 0;
    }
    vector<int> peaks;
    peaks.reserve(2 * s * s);   // shouldn't be more than that, but may be..
    // and then find the peaks.. 
    if(v[0] > v[1]){
	peaks.push_back(0);
    }
    for(uint i=1; i < v.size() - 1; i++){
	if(v[i] > v[i-1] && v[i] > v[i+1]){   // the minimal definition of a peak
	    peaks.push_back(i);
	}
    }
    // and the last position is a special case.. again..
    if(v.back() > v[v.size() - 2]){
	peaks.push_back(v.size() - 1);
    }
    // and then just add up the different positions..
    for(uint i=1; i < peaks.size(); i++){
	if(peaks[i] - peaks[i-1] < dist.size()){
	    dist[peaks[i] - peaks[i-1]] += 1.0;
	}
    }
    // and that is where we end up.. 
}

void dropVolume::evaluateGaussianModel(float xVar, float yVar, float zVar, float maxExcess){
    // use a gaussian model to estimate the pointsource intensity.. 
    
    gm_xvar = xVar;
    gm_yvar = yVar;
    gm_zvar = zVar;
    // first we need to find out our own center (or rather an estimate of our center position
    float xc, yc, zc;
    // and our center peak value..
    float pv;
    // for which we use a kernel smoothing algorithm using a 
    // variance of 2.5 (and a gaussian kernel function).
    float var = 2.5;

    // in fact this will fail when we are sufficiently close to a big signal, such that the peak
    // position is from a neighbouring voxel..
    //
    // this doesn't happen too often however, and I think that for now we can kind of ignore it.
    // It shouldn't be too difficult to fix this though.. hmm
    findCenter(pv, xc, yc, zc, var);
    x_center = xc;
    y_center = yc;
    z_center = zc;      // I know this is a bit ugly, -- should probably set these in setShape, but .. 
                        // should move the function at some point in the future.. 
    
//    cout << "setting center : " << x_center << "," << y_center << "," << z_center << "\tvariances : " << gm_xvar << "," << gm_yvar << "," << gm_zvar << endl;

    // assume that there is a point source at xc, yc, zc that decreases with distance from the source
    // in a gaussian manner  exp(-(d*d)/var), where d is the distance from the source and var is the
    // variance (i.e. square of the standard deviation).

    // since euclidean distances are additive in the squares of their components, we can combine the effect
    // the three individual dimensions by multiplying their components, which allows us to use a different
    // variance for each term.

    // go through a number of voxels in the voxel volume. Since neigbhouring voxels will contain degraded signals
    // from other sources (plus some background), they can always be higher than than that provided for by the 
    // gaussian model, however, then can not be any lower. Hence we can calculate a maximum allowable value for the
    // point source that we are considering.

    // in other words a single pixel that is high will be decreased due to the lack of appropriate high values around it..
    
    // the main issue here is how many pixes to choose and how strict to be...
    
    int pr = radius /2;
    pr = pr < 1 ? 1 : pr;
    int sl = radius * 2 + 1;
    int Sl = sl * sl;
    int psl = pr * 2 + 1;
    vector<float> ratios;
    ratios.reserve(psl * psl * psl);
    // and we are assuming that this will include the center, but it doesn't actually matte that much. 
    for(int z=-pr; z <= pr; z++){
	int zp = z + radius;
	float zd = (float(zp) + 0.5) - zc;
	zd = zd * zd;
	for(int y=-pr; y <= pr; y++){
	    int yp = y + radius;
	    float yd = (float(yp) + 0.5) - yc;
	    yd = yd * yd;
	    for(int x=-pr; x <= pr; x++){
		int xp = x + radius;
		float xd = (float(xp) + 0.5) - xc;
		xd = xd * xd;
		float mv = pv * exp(-xd/xVar) * exp(-yd/yVar) * exp(-zd/zVar);
		float ratio = values[zp * Sl + yp * sl + xp] / mv;   // i.e. the number by which we have to multiply the thingy.. 
		ratios.push_back(ratio);     // which we'll then use to identify the thingy with.. 
	    }
	}
    }
    // in a strict sense, we would take just the minimum ratio and then multiply the peakValue by that, 
    // as if we were following a strict rule that would be the normal thing to do. However, that 
    // could quite easily be influenced by too many different things..
    
    // one way around is to do something like... 
    // let's take the lowest 10th percentile and use that to set the value that we are looking for..
    // multiply that by the thingy and we can do something a little interesting..
   
    sort(ratios.begin(), ratios.end());
    //   cout << "gaussian model totalValue : " << totalValue << "  -->  ";
    totalValue = pv;
    int prop = 3;
//     if(ratios[0] < 1.0){
// 	totalValue = pv * ratios[0];
//     }

    if(ratios[(ratios.size()-1) / prop] < 1.0){
	totalValue = pv * ratios[(ratios.size()-1) / prop];
    }
//    cout << totalValue << endl;
    // but the problem is that we can't see if this is any good or not.. 
}

void dropVolume::findCenter(float& peakValue, float& xCenter, float& yCenter, float& zCenter, float var){
    // find the center of the dropVolume by an estimate from the central lines.
    // Use these to create a smoothened curve, from which we can estimate everything..
    
    // first get the central lines...
    vector<float> xLine = centralLine(0);
    vector<float> yLine = centralLine(1);
    vector<float> zLine = centralLine(2);

    int s = xLine.size();   // the size..
    int smult = 10;   // the smultiplier
    int smoothSize = s * smult;

    // ok. let's make three different arrays..
    vector<float> xsm(smoothSize);
    vector<float> ysm(smoothSize);
    vector<float> zsm(smoothSize);

    float kernelSum = 0;

    float d;
    for(uint i=0; i < xLine.size(); i++){
	d = float(i) + 0.5;
	d = d * d;
	kernelSum += exp(-d/var);   // this is the amount by which each thingy will be multiplied..
    }
    for(uint i=0; i < xsm.size(); i++){
	xsm[i] = 0;
	ysm[i] = 0;
	zsm[i] = 0;
	for(uint j=0; j < xLine.size(); j++){
	    d = (float(i) + 0.5)/float(smult)  - (float(j) + 0.5);
	    d = d * d;
	    xsm[i] += xLine[j] * exp(-d/var);
	    ysm[i] += yLine[j] * exp(-d/var);
	    zsm[i] += zLine[j] * exp(-d/var);
	}
    }
    xcline = xsm;
    ycline = ysm;
    zcline = zsm;

    // and now go through and find the center position for the three lines.
    // note that the peakValue we set will be the highest of the three peak values
    // divided by the thingy.. 

    // this is just an estimate however.. !
    int xPeakPos, yPeakPos, zPeakPos;
    xPeakPos = yPeakPos = zPeakPos = 0;
    float xpv, ypv, zpv;
    xpv = xsm[0];
    ypv = ysm[0];
    zpv = zsm [0];
    for(uint i=0; i < xsm.size(); i++){
	if(xpv < xsm[i]){ xpv = xsm[i]; xPeakPos = i; }
	if(ypv < ysm[i]){ ypv = ysm[i]; yPeakPos = i; }
	if(zpv < zsm[i]){ zpv = zsm[i]; zPeakPos = i; }
    }
    // if peakPosition is not really in the center area, then we've probably screwed up, but I think that might end up being alright..
    xCenter = (float(xPeakPos) + 0.5) / float(smult);
    yCenter = (float(yPeakPos) + 0.5) / float(smult);
    zCenter = (float(zPeakPos) + 0.5) / float(smult);

    // note that the peak values are calculated for    xpv      xCenter, radius, radius
    //                                                 ypv      radius, yCenter, radius
    //                                                 zpv      radius, radius, zCenter
    
    // where radius should really be radius + 0.5
    
    // this means that the real peak value is at some distance from there, so 
    // we should compensate for that..
    float rf = float(radius) + 0.5;
    float xcomp = exp(-((rf - yCenter) * (rf - yCenter) )/gm_yvar) * exp(-((rf - zCenter) * (rf - zCenter))/gm_zvar);
    float ycomp = exp(-((rf - xCenter) * (rf - xCenter) )/gm_xvar) * exp(-((rf - zCenter) * (rf - zCenter))/gm_zvar);
    float zcomp = exp(-((rf - yCenter) * (rf - yCenter) )/gm_xvar) * exp(-((rf - xCenter) * (rf - xCenter))/gm_xvar);
    
    // compensate the values, and then take the biggest one ?? 
    // 
    float comp_xpv = xpv / xcomp;
    float comp_ypv = ypv / ycomp;
    float comp_zpv = zpv / zcomp;

//    cout << "xpv : " << xpv << " -> " << comp_xpv << " ypv : " << ypv << " -> " << comp_ypv << "  zpv : " << zpv << " -> " << zpv << endl;

    peakValue = comp_xpv > comp_ypv ? comp_xpv : comp_ypv;
    peakValue = comp_zpv > peakValue ? comp_zpv : peakValue;
    peakValue = peakValue / kernelSum;   // necessary since we multiply everything by this.. 
    // which is all we really need to do for this.. 

}


void dropVolume::growKernel(int x, int y, int z, int dx, int dy, int dz, float mv){


    int d = radius * 2 + 1;
    int count = 0;
    if(x < 0 || x >= d || y < 0 || y >= d || z < 0 || z >=d){
	return;
    }
    int a = d * d;    // for conveniece.. 
    int cp = z * a + y * d + x;

    int nx, ny, nz;   // the new values..
    nx = x + dx;
    ny = y + dy;
    nz = z + dz;

    // cout << "\tgrowKernel " << x << ", " << y << ", " << z << "  with " << dx << ", " << dy << ", " << dz << "\tvalue : " << values[cp] << " : " << mv 
//	 << "\t ratio: " << values[cp] / mv << endl;
    if(values[cp] < mv){
	return;
    }
    kernelValues[cp] = values[cp];     // this one also gets added if we add any neighbours, so we are not dependent on the value of a single voxel.

    // use an array of d values instead and loop through all combinations..
    // 7 distinct possibilities
    if(dz && dy && dx){
	// 3d corner... -- check all 
	// in a 3d corner I have seven neigbhours that have to be checked.. (since only one direction for each dimension).
	//cout << "\t\t3d corner " << endl;
	growKernel(nx, ny, nz, dx, dy, dz, mv);
	growKernel(nx, ny, z, dx, dy, 0, mv);
	growKernel(nx, y, nz, dx, 0, dz, mv);
	growKernel(nx, y, z, dx, 0, 0, mv);
	growKernel(x, ny, nz, 0, dy, dz, mv);
	growKernel(x, ny, z, 0, dy, 0, mv);
	growKernel(x, y, nz, 0, 0, dz, mv);
	// and that should be it I think.. 
	return;
    }
    if(dz && dy){
	// 2 d corner
	//cout << "\t\t2d corner z & y" << endl;
	growKernel(x, ny, nz, 0, dy, dz, mv);
	growKernel(x, ny, z, 0, dy, 0, mv);
	growKernel(x, y, nz, 0, 0, dz, mv);
	return;
    }
    if(dz && dx){
	// 2d corner
	//cout << "\t\t2d corner z & x" << endl;
	growKernel(nx, y, nz, dx, 0, dz, mv);
	growKernel(nx, y, z, dx, 0, 0, mv);
	growKernel(x, y, nz, 0, 0, dz, mv);
	return;
    }
    if(dy && dx){
	// 2d corner
	//cout << "\t\t2d corner y & x" << endl;
	growKernel(nx, ny, z, dx, dy, 0, mv);
	growKernel(nx, y, z, dx, 0, 0, mv);
	growKernel(x, ny, z, 0, dy, 0, mv);
	return;
    }
    if(dz){
	// just a row..
	//cout << "\t\tz row" << endl;
	growKernel(x, y, nz, 0, 0, dz, mv);
	return;
    }
    if(dy){
	// just a row..
	//cout << "\t\ty column" << endl;
	growKernel(x, ny, z, 0, dy, 0, mv);
	return;
    }
    if(dx){
	// again just a row..
	//cout << "\t\tx row" << endl;
	growKernel(nx, y, z, dx, 0, 0, mv);
	return;
    }
}

vector<float> dropVolume::centralLine(uint dim){
    int s = 1 + radius * 2;
    int S = s * s;
    vector<float> line(s, 0);
    if(dim > 2){
	cerr << "dropVolume centralLine dimension too high : " << endl;
	return(line);
    }
    if(values.size() != S * s){
	cerr << "centralLine values size is not : " << s * S << endl;
	return(line);
    }
    int x, y, z;   // the coordinates..
    x = y = z = radius;  // the center position.
    for(int i=0; i < s; i++){
	switch(dim){
	    case 0 :
		x = i;
		break;
	    case 1 :
		y = i;
		break;
	    default :
		z = i;
	}
	// then line[i] is just whatever happens to lie at the appropriate position..
	line[i] = values[z * S + y * s + x];
    }
    return(line);
}

void dropVolume::printShapes(){
    cout << "X shape : ";
    for(uint i=0; i < x_shape.size(); i++){
	cout << "\t" << x_shape[i];
    }
    cout << endl << "Y shape : ";
    for(uint i=0; i < y_shape.size(); i++){
	cout << "\t" << y_shape[i];
    }
    cout << endl << "Z shape : ";
    for(uint i=0; i < z_shape.size(); i++){
	cout << "\t" << z_shape[i];
    }
    cout << endl << endl;
}

linearPeaks::linearPeaks(int W, int H, std::vector<voxel> xpeaks, std::vector<voxel> ypeaks){
    xPeaks = xpeaks;
    yPeaks = ypeaks;
    w = W;
    h = H;
    // this is assuming that the xpositions and ypositions are given in the same coordinate manner..
    int px, py;
    px = py = 0;
    sort(xPeaks.begin(), xPeaks.end());
    sort(yPeaks.begin(), yPeaks.end());
    while(px < xPeaks.size() && py < yPeaks.size()){
	if(xPeaks[px].pos == yPeaks[py].pos){
	    xyPeaks.push_back(xPeaks[px]);
	    ++px;
	    ++py;
	    continue;
	}
	if(xPeaks[px].pos < yPeaks[py].pos){
	    ++px;
	    continue;
	}
	++py;
    }
    cout << "size of merged Peaks : " << xyPeaks.size() << endl;
}    

vector<int> linearPeaks::x_line_peaks(int ypos){
    return(x_positions(xPeaks, ypos));
}

vector<int> linearPeaks::y_line_peaks(int xpos){
    return(y_positions(yPeaks, xpos));
}

vector<int> linearPeaks::mX_line_peaks(int ypos){
    return(x_positions(xyPeaks, ypos));
}

vector<int> linearPeaks::mY_line_peaks(int xpos){
    return(y_positions(xyPeaks, xpos));
}

vector<int> linearPeaks::x_positions(vector<voxel> peaks, int ypos){
    vector<int> Peaks;
    int b = ypos * w;
    int e = b + w;
    for(uint i=0; i < peaks.size(); i++){
	if(peaks[i].pos >= e){
	    break;
	}
	if(peaks[i].pos < b){
	    continue;
	}
	Peaks.push_back(peaks[i].pos % w);
    }
    return(Peaks);
}

vector<int> linearPeaks::y_positions(vector<voxel> peaks, int xpos){
    // a little bit more difficult but not so bad..
    vector<int> Peaks;
    for(int i=0; i < peaks.size(); i++){
	if(peaks[i].pos % w == xpos){
	    Peaks.push_back(yPeaks[i].pos / w);
	}
    }
    return(Peaks);
}

void linearPeaks::printStats(){
    cout << "Row peaks : " << xPeaks.size() << "  Col peaks : " << yPeaks.size() << "  merged Peaks : " << xyPeaks.size() << endl;
}

void line::init(){
    rn = start / width;
    col_start = start % width;
    col_stop = stop % width;
}

int line::neighborsBelow(line& b){
    // and then simply..
    if(b.rn <= rn){
	return(-1);
    }
    if(b.rn > rn + 1){
	return(1);
    }
    if(b.col_stop < col_start){
	return(-1);
    }
    if(b.col_start > col_stop){
	return(1);
    }
    // and if we are here, then surely we overlap..
    return(0);
    // this could be written shorter, but the way we are going to use this makes the first condition the most common
    // I think it may make more sense to write like this.
}

void line::neighboringLines(vector<line>& lines, set<line>& nLines){
    // this is horribly inefficient and not very elegant.
    // I'm not feeling very smart today.. -- hope to rewrite sometime.. 


    // the lines have to be sorted but don't sort them here, as we are going to call this function 
    // recursively.. 
    // and now find myself in the vector..
    int selfIndex = -1;
    for(uint i=0; i < lines.size(); i++){
	if(lines[i].start == start){    // bit of a kludge, but..
	    selfIndex = i;
	    break;
	}
    }
    //cout << "selfIndex is : " << selfIndex << endl;
    if(selfIndex < 0){
	cout << "line::neighboringLines no line with start position : " << start << "  in list given" << endl;
	return;
    }
    nLines.insert(lines[selfIndex]);   // which must invoke some sort of copy, but.. 
//    cout << "inserted myself" << endl;
    vector<line> n_lines;
    // the look for lines above that might be neighbouring..
    for(int i=selfIndex-1; i >= 0; i--){
//	cout << "looking below i " << i << endl;
	if(rn > lines[i].rn + 1){
	    break;
	}
	if(!lines[i].neighborsBelow(lines[selfIndex])){
	    if(!nLines.count(lines[i])){
		nLines.insert(lines[i]);
		n_lines.push_back(lines[i]);
	    }
	}
    }
    // then look for lines below that might be neighbouring.. 
    for(int i=selfIndex+1; i < lines.size(); i++){
//	cout << "looking above i " << i << endl;
	if(lines[i].rn > rn + 1){
	    break;
	}
	if(!neighborsBelow(lines[i])){
	    if(!nLines.count(lines[i])){
		nLines.insert(lines[i]);
		n_lines.push_back(lines[i]);
	    }
	}
    }
//    cout << "total of " << nLines.size() << " lines in thingy " << endl
//	 << "current neighgours " << n_lines.size() << endl;
    // by which time we now have all the lines that neighbours the present line.
    // however to find the ones that neighbours the other ones we can simply call this function
    // recursively. However, I'm rather afraid that will be really slow.. 
    for(uint i=0; i < n_lines.size(); i++){
//	cout << "\t" << i << endl;
	n_lines[i].neighboringLines(lines, nLines);
    }
}

void Nucleus::incrementSignal(float* source, int width, int height){
    // the source has to be big enough to hold all of the data. 
    // we need to know the width of the field in order to know how 
    // the source is arranged.

    // first make a set of boundary elements so that we can easily go through everything..
    set<twoDPoint> pointSet;
    for(uint i=0; i < perimeter.size(); i++){
	pointSet.insert(perimeter[i]);
    }
    // then go through and check..
    int x_beg = min_x > 0 ? min_x -1 : 0;
    int y_beg = min_y > 0 ? min_y -1 : 0;
    int x_end = max_x < width -1 ? max_x + 1 : width - 1;
    int y_end = max_y < height -1 ? max_y + 1 : height - 1;
    // which should be ok.. ... 
    for(int dy=y_beg; dy <= y_end; dy++){
	bool inside = false;
	for(int dx=x_beg; dx <= x_end; dx++){
	    if(pointSet.count(twoDPoint(dx, dy))){
		inside = !inside;
	    }
	    if(inside){
		totalSignal += source[width * dy + dx];
	    }
	}
    }
}

void Nucleus::smoothenPerimeter(){
    smoothPerimeter.resize(perimeter.size());
    // and then we need to go through and assign the values using a smoothening parameters..
    int smlength = 20;
    for(uint i=0; i < perimeter.size(); i++){
	// set points i to i + 10 to a straight line between 
	// point i and point i + 10;
	// so make a line between .. 
	int x1 = perimeter[i].x;
	int x2 = perimeter[(i + smlength) % perimeter.size()].x;
	int y1 = perimeter[i].y;
	int y2 = perimeter[(i + smlength) % perimeter.size()].y;
	int j = smlength/2;
//	for(int j=smlength/2; j < smlength; j++){
	    int p = (i + j) % perimeter.size();   // which makes it go around in a circle.. 
	    smoothPerimeter[p].x = x1 + (j * (x2 - x1))/smlength;
	    smoothPerimeter[p].y = y1 + (j * (y2 - y1))/smlength;
//	}
    }
}

vector<float> Nucleus::inversionScores(float* source, int width, int height){
    int blength = 6;   // bridge length,, we can change this later on, but..
    float maxScore = 0;
    vector<float> v(perimeter.size());
    for(uint i=0; i < perimeter.size(); i++){
	// base this on the angle between i, i + blength annd i + blength * 2;
	// considering the angle at corner A in triangle C-A-B
	int C = i;      
	int A = (i + blength) % perimeter.size();
	int B = (i + 2 * blength) % perimeter.size();
	int Cx = perimeter[C].x;
	int Cy = perimeter[C].y;
	int Ax = perimeter[A].x;
	int Ay = perimeter[A].y;
	int Bx = perimeter[B].x;
	int By = perimeter[B].y;

	// cosine rule states that :
	// 
	// a^2 = b^2 + c^2 - 2bcCosA
	// 
	// from a triangle with corners A, B and C
	// where a, b and c are the lengths of the lines opposing the corresponding
	// corner..
	//
	// hence to get CosA ...
	//
	// CosA = (b^2 + c^2 - a^2) / (2 * b * c);
	//
	// first calculate the lengths a, b and c using simple stuff..
	// 
	// a is the length between points C and B
	double a = sqrt(double( (Bx - Cx)*(Bx - Cx) + (By - Cy)*(By - Cy) ));
	// b is the length between points A and C
	double b = sqrt(double( (Ax - Cx)*(Ax - Cx) + (Ay - Cy)*(Ay - Cy) ));
	// c is the length betweeen points A and B
	double c = sqrt(double( (Ax - Bx)*(Ax - Bx) + (Ay - By)*(Ay - By) ));
	
	double cosA = (b*b + c*c - a*a)/(2 * b * c);
	
	if(a && b && c){
	    v[A] = 1.0 + float(cosA);
	}else{
	    v[A] = 0;
	}
	if(maxScore < v[A]){
	    maxScore = v[A];
	}

	// which is nice.. since a small angle means an inversion and small angles
	// give high cosine values (0 --> 1, 90 --> 0, and 180 --> -1) high values will indicate
	// inversions.. nice.. 


// 	int x1 = perimeter[i].x;
// 	int x2 = perimeter[(i + blength) % perimeter.size()].x;
// 	int y1 = perimeter[i].y;
// 	int y2 = perimeter[(i + blength) % perimeter.size()].y;
// 	// This is to say that we are taking a line between points 1 and 2.
// 	// This would be easy except that we are dealing with pixels, not real space values
// 	// so we have to approximate in some way. Doing it properly isn't worth the hassle..
	
// 	// divide the distance into a number of steps, (making this 2 times the integer distance)
// 	int divs = int(2 * sqrt(double((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2))));
// 	//cout << endl << "making line from : " << x1 << "," << y1 << " --> " << x2 << "," << y2 << endl;
// 	twoDPoint begin(x1, y1);
// 	twoDPoint end(x2, y2);
// 	twoDPoint current(x1, y1);
// 	int pNo = 0;
// 	float meanValue = 0;
// 	float priorValues = 0;
// 	float afterValues = 0;
// 	for(int j=1; j < divs; j++){
// 	    int x = x1 + (j * (x2 - x1))/divs;
// 	    int y = y1 + (j * (y2 - y1))/divs;
// 	    int px = x1 - (j * (x2 - x1))/divs;
// 	    int py = y1 - (j * (y2 - y1))/divs;
// 	    int ax = x2 + (j * (x2 - x1))/divs;
// 	    int ay = y2 + (j * (y2 - y1))/divs;
// 	    twoDPoint thisPoint(x, y);
// 	    if(thisPoint == current){
// 		continue;
// 	    }
// 	    if(thisPoint == end){
// 		break;
// 	    }
// 	    // cout << "\t" << x << "," << y << endl;
// 	    // and if not the case then increment something.. and 
// 	    pNo++;
// 	    meanValue += source[y * width + x];
// 	    priorValues += source[py * width + px];
// 	    afterValues += source[ay * width + ay];
// 	    current = thisPoint;
// 	}
// 	v[i] = (afterValues + priorValues)/(2.0 * meanValue);
// //	meanValue = meanValue/float(pNo);   // which is reasonable.. after all..
// //	v[i] = (source[y1 * width + x1] + source[y2 * width + x2])/(float(2) * meanValue);   // which might look good with a log 2 as well.. 
// 	if(v[i] != v[i]){
// //	    cout << "\t\t1 : " << source[y1 * width + x1] << "\t2 : " << source[y2 * width + x2] << "\tmeanValue : " << meanValue << endl;
// //	    cout << "\t\t score : " << v[i] << endl;
// 	    v[i] = 1.0;
// 	}
// 	if(v[i] > maxScore){
// 	    maxScore = v[i];
// 	}

    }
    cout << "Max Inversion Score : " << maxScore << endl;
    inversions = v;
    return(v);
}
