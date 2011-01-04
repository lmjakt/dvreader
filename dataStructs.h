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

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <qcolor.h>
#include <string.h>
#include <math.h>

//typedef unsigned int uint;

typedef unsigned long o_set;

struct pos {
  int x, y, z;
  pos(int xp, int yp, int zp){
    x = xp; y = yp; z = zp;
  }
  pos(uint xp, uint yp, uint zp){
    x = (int)xp; y = (int)yp; z = (int)zp;
  }
  pos(float xp, float yp, float zp){
    x = (int)(roundf(xp));
    y = (int)(roundf(yp));
    z = (int)(roundf(zp));
  }
  pos(){
    x = y = z = 0;
  }
  friend bool operator <(const pos& a, const pos& b){
    if(a.z != b.z)
      return(a.z < b.z);
    if(a.y != b.y)
      return(a.y < b.y);
    return(a.x < b.x);
  }
  friend bool operator ==(const pos& a, const pos& b){
    return( a.z == b.z && a.y == b.y && a.x == b.x);
  }
  friend pos operator +(const pos& a, const pos& b){
    pos c(a.x + b.x, a.y + b.y, a.z + b.z);
    return(c);
  }
  pos operator -(pos a){
    a.x = x - a.x;
    a.y = y - a.y;
    a.z = z - a.z;
    return(a);
  }
  friend std::ostream& operator<<(std::ostream& out, const pos& a){
    return( out << a.x << "," << a.y << "," << a.z );
  }
};


struct fluorInfo {  // information about fluoresence
  float excitation;
  float emission;
  float exposure;
  fluorInfo(){
    excitation = emission = exposure = 0;
  }
  fluorInfo(float ex, float em, float exp){
    excitation = ex;
    emission = em;
    exposure = exp;
  }
  // and a sorting operator
  friend bool operator <(const fluorInfo& a, const fluorInfo& b){
    if(a.emission == b.emission && a.excitation == b.excitation)
      return(a.exposure < b.exposure);
    if(a.emission == b.emission)
      return(a.excitation < b.excitation);
    return(a.emission < b.emission);
  }
  friend bool operator ==(const fluorInfo& a, const fluorInfo& b){
    return(a.emission == b.emission && a.excitation == b.excitation && a.exposure == b.exposure);
  }
  friend bool operator !=(const fluorInfo& a, const fluorInfo& b){
    return(!(a == b));
  }
};


struct backgroundPars {
  uint x_m, y_m, z_m;
  float pcntile;
  backgroundPars(){
    x_m = y_m = 32;
    z_m = 8;
    pcntile = 50;
  }
  backgroundPars(uint x, uint y, uint z, float p){
    x_m = x; y_m = y; z_m = z;
    pcntile = (p > 0 && p < 100) ? p : 50;
  }
  backgroundPars(int x, int y, int z, float p){
    x_m = (uint)x; y_m = (uint)y; z_m = (uint)z;
    pcntile = (p > 0 && p < 100) ? p : 50;
  }
};


struct objectDistanceInfo {
    bool isFlat;
    int geneNo;
    float sigma;
    float order;
    std::vector<int> experiments;
    std::vector<std::vector<float> > values;
    objectDistanceInfo(std::vector<int> e, std::vector<std::vector<float> > v, bool isflat=false){
	experiments = e;
	values = v;
	isFlat = isflat;
	geneNo = 0;
	sigma = 0;
	order = 0;
    }
    objectDistanceInfo(){
    }
                                                                                                               
};


struct PointCoordinate {
  int index;     // the experimental index.. if not defined, then do something else..
  float* coords;
  int dimNo;
  //  vector<float> coords;       // the coordinates, -- use a vector, so we can use different dimensional space
  PointCoordinate(){
    // do nothing..
  }
  PointCoordinate(int i, float* crds, int n){
    index = i;
    coords = crds;
    dimNo = n;
  }
};


struct parameterData {
    float* values;
    float maxValue, minValue;
    unsigned int width, height;
    int xOrigin, yOrigin;
    parameterData(){
	values = 0;
	width = height = 0;
	xOrigin = yOrigin = 0;
    }
    parameterData(float* v, unsigned int w, unsigned int h){
	if(w > 0 && h > 0){ 
	    values = v;
	    width = w;
	    height = h;
	    xOrigin = yOrigin = 0;  // default to bottom left corner.. 
	    maxValue = minValue = values[0];
	    for(uint i=0; i < width * height; ++i){
		if(values[i] > maxValue)
		    maxValue = values[i];
		if(values[i] < minValue)
		    minValue = values[i];
	    }
	    std::cout << "parameterData constructor range is : " << minValue << " --> " << maxValue << std::endl;
	}else{
	    values = 0;
	    width = height = 0;
	    delete v;
	}
    }
    ~parameterData(){
	if(values){
//	    delete values;
	    // if we are going to do this we need to count how many references to values ?
	}
    }
};

struct offsets {
    int dx, dy;
    float corr;
    float norm_corr;
    offsets(){
	dx = dy = 0;
	corr = -2;
	norm_corr = -2;
    }
    offsets(int x, int y, float c){
	dx = x;
	dy = y;
	corr = c;
	norm_corr = -2;
    }
};

struct overlap_data {
    float x, y;   // the coordinates of this panel
    float nx, ny; // the corrdinates of the neighbour
    int px, py;   // the panel coordinates
    int pnx, pny; // and of the neighbour
    unsigned int width;
    unsigned int height;  // the size of the images stored..
    int dx, dy;           // the offset of b over a.. 
    float* a;
    float* b;     // the images themselves
    offsets offsts;  // offset information.. 
    
    overlap_data(){
	x = y = 0;
	width = height = 0;
	a = b = 0;
	dx = dy = 0;
    }
    overlap_data(float X, float Y, float NX, float NY, int PX, int PY, int PNX, int PNY, unsigned int W, unsigned int H, int delta_x, int delta_y, float* imageA, float* imageB, offsets off){
	x = X; y = Y;
	nx = NX; ny = NY;
	px = PX; py = PY;
	pnx = PNX; pny = PNY;
	width = W;
	height = H;
	dx = delta_x;
	dy = delta_y;
	a = imageA;
	b = imageB;
	offsts = off;
    }
    ~overlap_data(){
	if(a){
	    delete a;
	}
	if(b){
	    delete b;
	}
    }
    
};

struct raw_data {
    float** values;
    unsigned int channels;
    unsigned int length;
    unsigned int* positions;
    raw_data(unsigned int channelNo, unsigned int size){
	values = new float*[channelNo];
	positions = new unsigned int[channelNo];
	channels = channelNo;
	length = size;
	for(unsigned int i=0; i < channels; i++){
	    values[i] = new float[size];
	    memset(values[i], 0, sizeof(float) * length);
	    positions[i] = 0;
	}
    }
    ~raw_data(){
	for(uint i=0; i < channels; i++){
	    delete []values[i];
	}
	delete []values;
	delete []positions;
    }
};

struct view_area {
    float x, y, w, h;              // real position coordinates..
    int px, py, pw, ph;   // pixel coordinates
//    unsigned int px, py, pw, ph;   // pixel coordinates
    float xscale, yscale;

    view_area(){
	x = y = w = h = 0;
	px = py = pw = ph = 0;
    }
    void setArea(float xf, float yf, float wf, float hf, int xi, int yi, int wi, int hi){
//    void setArea(float xf, float yf, float wf, float hf, unsigned int xi, unsigned int yi, unsigned int wi, unsigned int hi){
	x = xf; y = yf; w = wf; h = hf;
	px = xi; py = yi; pw = wi; ph = hi;
	xscale = w/(float)pw;
	yscale = h/(float)ph;
    }
};

struct color_map {
    float r, g, b;
    color_map(){
	r = g = b = 1.0;
    }
    color_map(float rc, float gc, float bc){
	r = rc; g = gc; b = bc;
    }
    color_map(QColor c){
	int ri, gi, bi;
	c.getRgb(&ri, &gi, &bi);
	r = float(ri)/255.0;
	g = float(gi)/255.0;
	b = float(bi)/255.0;
    }
    void set_color(float& rc, float& gc, float& bc){
	rc = r; gc = g; bc = b;
    }
    QColor qcolor(){
	return(QColor(int(r * 255.0), int(g * 255.0), int(b * 255.0)));
    }
};

struct channel_info {
  fluorInfo finfo;
  color_map color;
  float maxLevel;
  float bias;
  float scale;
  bool bg_subtract;
  backgroundPars bgPar;
  bool contrast;
  bool include;
  channel_info(){
    maxLevel = 4096;
    bias = 0;
    scale = 1.0;
    include = true;
    bg_subtract = contrast = false;
  }
  channel_info(color_map cm, float ml, float b, float s, bool bgs, bool cnt){
    color = cm; maxLevel = ml; bias = b; scale = s; 
    bg_subtract = bgs; contrast = cnt;
    include = true;
  }
  friend bool operator <(const channel_info& a, const channel_info& b){
    if(a.finfo != b.finfo)
      return(a.finfo < b.finfo);
    if(a.maxLevel != b.maxLevel)
      return(a.maxLevel < b.maxLevel);
    if(a.bias != b.bias)
      return(a.bias < b.bias);
    return(a.scale != b.scale);
  }
    
};

struct distribution {   // a struct that holds the distribution for some values..
    std::vector<float> counts;   // not actual counts but partial counts..
    float minValue;
    float maxValue;         // the min and max values for the distribution..
    distribution(){
	minValue = maxValue = 0;
    }
    distribution(std::vector<float> values, uint bno);  // basically calculate the distribution for this..
    void printDistribution();
};


struct voxel {
    int pos;
    float value;
    voxel(){
	pos = 0;
	value = 0;
    }
    voxel(int p, float v){
	pos = p;
	value = v;
    }
    friend bool operator <(const voxel& a, const voxel& b){
	return(a.pos < b.pos);
    }
};

struct linearPeak {   // information about a linear peak.
    long position;
    long begin;
    long end;   // the offset ... use a long, we can then use this in a much larger expanded picture.. 
    int dim;    // the dimension, 0, 1 or 2, X, Y, or Z
    int peak_width;  // only end - begin if dim = 0
    float peak_height;
    float pb_height, pe_height;   // begin end values of peak;
    
    linearPeak(){
	begin = end = 0;
	dim = -1;
	peak_height = pb_height = pe_height = 0;
    }
    linearPeak(long peakPosition, long peak_begin, long peak_end, int dimension, int peakWidth, float peakHeight, float peakBeginHeight, float peakEndHeight){
	position = peakPosition;
	begin = peak_begin;
	end = peak_end;
	peak_width = peakWidth;
	dim = dimension;    // but this is just for convenience, as its actually governed by peak_begin and peak_end
	peak_height = peakHeight;
	pb_height = peakBeginHeight;
	pe_height = peakEndHeight;
    }
    void offset(long of){
	position += of;
	begin += of;
	end += of;
    }
    void setPos(long pos, long multiplier){
	long bd = position - begin;
	long ed = end - position;
	position = pos;
	begin = position - (bd * multiplier);
	end = position + (ed * multiplier);
    }
};


struct linearPeaks {
/*     linearPeaks(int W, int H, std::vector<voxel> xpeaks, std::vector<voxel> ypeaks, std::vector<int> xypeaks){ */
/* 	w = W; */
/* 	h = H; */
/* 	xPeaks = xpeaks; */
/* 	yPeaks = ypeaks; */
/* 	xyPeaks = xypeaks; */
/*     } */
    linearPeaks(){
	w = h = 0;
    }
    linearPeaks(int W, int H, std::vector<voxel> xpeaks, std::vector<voxel> ypeaks);   // computes the xypeaks.. itself.. 
    std::vector<int> x_line_peaks(int ypos);   // give the x-positions for a given y positions
    std::vector<int> y_line_peaks(int xpos);   // give the y-positions for a given x position
    std::vector<int> mX_line_peaks(int ypos);  // give the merged peaks for a given x position
    std::vector<int> mY_line_peaks(int xpos);
    void printStats();
    
    std::vector<voxel> xPeaks;
    std::vector<voxel> yPeaks;
    std::vector<voxel> xyPeaks;

    private :
    std::vector<int> x_positions(std::vector<voxel> peaks, int ypos);
    std::vector<int> y_positions(std::vector<voxel> peaks, int xpos);
    int w;
    int h;   // the dimensions..

};

struct simple_drop {
    int x, y, z;       // the peak position, not necessarily the center
    int xb, yb, zb;    // the begin positions
    int xe, ye, ze;    // the end positions.
    int radius;        // always have the same size, but values outside of xb, xe and so on are set to 0. (this allows us to do stuff).
    int id;
    
    int waveIndex;
    float waveLength;

    // we may also want to include the actual voxel data;
    std::vector<float> values;   // values starting from begin to end in the various dimensions. 
    float peakValue;
    float sumValue;
    float background;            // define in appropriate manner.
    
    simple_drop(){
	x = y = z = 0;
	radius = 0;
	id = -1;
    }
    simple_drop(int r, int xc, int yc, int zc, int xbegin, int xend, int ybegin, int yend, int zbegin, int zend, std::vector<float>& v, int wi, int wl){
	radius = r;
	x = xc; y = yc; z = zc;
	xb = xbegin; yb = ybegin; zb = zbegin;
	xe = xend; ye = yend; ze = zend;
	values = v;
	id = -1;
	waveIndex = wi;
	waveLength = wl;
	// values size should be (r *2 ) + 1
	unsigned int d = r * 2 + 1;
	if(values.size() != d * d * d){
	    std::cerr << "simple_drop constructor vector of values has bad size : " << values.size() << "  should be : " << d * d * d << std::endl;
	    values.resize(d * d * d);
	    for(uint i=0; i < values.size(); i++){
		values[i] = 0;
	    }
	}
	// simple sum values should be ok..
	sumValue = 0;
	peakValue = 0;
//	std::cout << "simple_drop constructor checking values" << std::endl;
	for(uint i=0; i < values.size(); i++){             // it might be better to use only those values that fall within the peak.. but..
	    sumValue += values[i];
	    if(peakValue < values[i])
		peakValue = values[i];
//	    std::cout << values[i] << "   ";
	}
//	std::cout << std::endl;
    }
    bool overlaps(simple_drop& dr){
      return( (xb < dr.xe) != (xe < dr.xb) &&
	      (yb < dr.ye) != (ye < dr.yb) &&
	      (zb < dr.ze) != (ze < dr.zb));
    }
    float peak_distance(simple_drop& dr){
	return(sqrt( float( (x - dr.x)*(x - dr.x) + (y - dr.y)*(y - dr.y) + (z - dr.z)*(z - dr.z) )));
    }
    friend bool operator <(const simple_drop& a, const simple_drop& b){
	if(a.waveLength != b.waveLength)
	    return(a.waveLength < b.waveLength);
	if(a.z != b.z)
	    return(a.z < b.z);
	if(a.y != b.y)
	    return(a.y < b.y);
	if(a.x != b.x)
	    return(a.x < b.x);
	return(a.sumValue < b.sumValue);
    }
    std::vector<float> centralLine(uint dim);
};

struct simple_drop_set {
    simple_drop_set(simple_drop* drop){
	drops.insert(drop);
    }
    ~simple_drop_set(){}
    void addDrop(simple_drop* drop){
	drops.insert(drop);
    }
    std::set<simple_drop*> drops;
};

struct dropVolume {      // though in fact we are going to define it in terms of a cube since this is much easier
    int center;
    int radius;   // which isn't really the radius, but the number of points to the left and right of the center
    std::vector<float> values;  // the values of all of the voxels in the cube.. (from -r,-r,-r to +r,+r,+r centered around center)
//    std::vector<float> gaussian_values;  // do a gaussian blur in 3 d (... oh dear.. )
    float backgroundMultiplier;    //the number of times the background something must be to be considered part of the core or kernel
    float totalValue;      // some kind of summation of the values.. 

    float xVariance, yVariance, zVariance;  // the variances for a gaussian model.. -- these are set in the setShapeFunction.. 
    float gm_xvar, gm_yvar, gm_zvar;        // the actual variances used in the gm (these are set externally as they are estimated
                                            // from the full set of things.. 
    float x_center, y_center, z_center;        // estimates for a center position (set in thingy).
    std::vector<float> xcline;
    std::vector<float> ycline;
    std::vector<float> zcline;      // the center lines (actually lenghtened ones, but ..)
    
    int x, y, z;          // the coordinates of the drop
    std::vector<float> x_shape;
    std::vector<float> y_shape;
    std::vector<float> z_shape;

    std::vector<float> x_peak_count;
    std::vector<float> x_peak_shape;
    std::vector<float> y_peak_count;
    std::vector<float> y_peak_shape;
    std::vector<float> z_peak_count;
    std::vector<float> z_peak_shape;     // these are shapes derived from the peak positions only
                                         // either by just adding 1 for a peak, or by adding some value
                                         // indicating the intensity at the peak position (minus say the trough value ?)
    // the core drifts across.. so let's look at peak distances...
    std::vector<float> x_peak_dist;
    std::vector<float> y_peak_dist;
    std::vector<float> z_peak_dist;   // 


    // ahh, the shapes don't look so good, so let's try something a bit different..
    float background;     // make this the mean of the lowest value of each X-scan
    float meanValue;
    float std;            // mean and standard deviation of all the stuff.
                          // This means that we can look at stuff in terms of standard deviations..
    // we'll also want a vector of floats for the kernel..
    std::vector<float> kernelValues;    // values not part of the kernel set to 0. Kernel 
                                        // distinguished in some kind of way related to background and other stuff. 
    

    dropVolume(){
	center = 0;
	radius = 0;
	backgroundMultiplier = 5.0;
	totalValue = 0;
	x = y = z = 0;
	xVariance = yVariance = zVariance = 0;
	gm_xvar = gm_yvar = gm_zvar = 0;
	x_center = y_center = z_center = 0;
    }
    dropVolume(int c, int r, int X, int Y, int Z, float bgm){
	center = c;
	radius = r;
	backgroundMultiplier = bgm;
	int d = r * 2 + 1;   // kind of the diameter.. 
	values.resize(d * d * d);
	for(uint i=0; i < values.size(); i++){
	    values[i] = 0;
	}
	totalValue = 0;
	x = X;
	y = Y;
	z = Z;
	// but let the user set the values.. -- don't use push !!
	xVariance = yVariance = zVariance = 0;
	gm_xvar = gm_yvar = gm_zvar = 0;
	x_center = y_center = z_center = 0;
    }
    void setShapes();
    void printShapes();
    void evaluateGaussianModel(float xVar, float yVar, float zVar, float maxExcess);
    void findCenter(float& peakValue, float& xCenter, float& yCenter, float& zCenter, float var);   // use a kernel smoothing to estimate te center position
    std::vector<float> centralLine(uint dim);   // give the central line for a given dimension (0 -> 2, 0 = x, 1 = y, 2 = z);
    private :
    // addPeakValues is actually called by setShapes only..  
    void addPeakValues(std::vector<float>& v, std::vector<float>& c_shape, std::vector<float>& v_shape);
    void addPeakDistances(std::vector<float>& v, std::vector<float>& dist);
    void growKernel(int x, int y, int z, int dx, int dy, int dz, float mv);    // adds values to the kernel,, calls itself.. 
};

struct threeDPeaks {
    std::vector<int> peaks;
    std::vector<dropVolume> drops;
    std::map<long, simple_drop> simpleDrops;
    std::map<long, linearPeak> xPeaks;
    std::map<long, linearPeak> yPeaks;
    std::map<long, linearPeak> zPeaks;
    float r, g, b;   // the colour of the model..
    int radius;      // the size of the drops. (This is gien as the number of voxels on either side of the center voxel)
    float minThreshold, maxThreshold;    // count as drop if totalValue within range.. 
    threeDPeaks(){
	r = g = b = 1.0;
	minThreshold = maxThreshold = 0;   // but later we can set this appropriately.. 
	radius = 4;                        // no reason, but I don't want to leave it at 0
    }
    threeDPeaks(int rad, int R, int G, int B){
	r = float(R)/255.0;
	g = float(G)/255.0;
	b = float(B)/255.0;
	maxThreshold = minThreshold = 0;
	radius = rad;
    }
};

struct twoDPoint {
    int x;
    int y;
    int id;
    twoDPoint(){
	x = y = 0;
	id = -1;
    }
    twoDPoint(int X, int Y){
	x = X;
	y = Y;
	id = -1;
    }
    twoDPoint(int X, int Y, int i){
	x = X;
	y = Y;
	id = i;
    }
    friend bool operator ==(const twoDPoint& a, const twoDPoint& b){
	return(a.x == b.x && a.y == b.y);
    }
    friend bool operator <(const twoDPoint& a, const twoDPoint& b){
	if(a.y == b.y){
	    return(a.x < b.x);
	}
	return(a.y < b.y);
    }
};

struct threeDPoint {
    int x, y, z;
    double xd, yd, zd;   // an expensive point, but with a fast distance determination..
    int id;
    threeDPoint(){
	x = y = z = 0;
	xd = yd = zd = 0.0;
	id = -1;
    }   
    threeDPoint(int X, int Y, int Z){
	x = X; y = Y; z = Z;
	xd = (double)x;
	yd = (double)y;
	zd = (double)z;
	id = -1;
    }
    threeDPoint(int X, int Y, int Z, int i){
	x = X; y = Y; z = Z;
	xd = (double)x;
	yd = (double)y;
	zd = (double)z;
	id = i;
    }
    threeDPoint(double X, double Y, double Z){
	xd = X;
	yd = Y;
	zd = Z;
	x = (int)X;
	y = (int)Y;
	z = (int)Z;
	id = -1;
    }
    double ed(threeDPoint& p){
	return(sqrt( (p.xd - xd)*(p.xd - xd) + (p.yd - yd)*(p.yd - yd) + (p.zd - zd)*(p.zd - zd)));
    }
    double sq_ed(threeDPoint& p){
	return( (p.xd - xd)*(p.xd - xd) + (p.yd - yd)*(p.yd - yd) + (p.zd - zd)*(p.zd - zd));
    }
    
    friend bool operator ==(const threeDPoint& a, const threeDPoint& b){
	return(a.x == b.x && a.y == b.y && a.z == b.z);
    }
};

struct line {
    int start;
    int stop;
    int rn;
    int col_start;  // row and column numbers.. 
    int col_stop;
    int width;   // this is the width.. of the field from which the line is taken.. 
    line(int st, int sp, int w){
	start = st;
	stop = sp;
	width = w;
	init();
    }
    line(){
	start = stop = 0;
	width = 1;
	init();
    }
    int neighborsBelow(line& b);   // returns -1, 0 or 1, where 0 indicates a neighbouring overlap.. (I know not great.. but)..
    void neighboringLines(std::vector<line>& lines, std::set<line>& nLines);   // given a set of lines, returns all lines that are neighbouring..
    friend bool operator <(const line& a, const line& b){
	return(a.start < b.start);
    }
    
    private :
    void init();
    
};

struct Nucleus {
    // a nucleus has a set of lines and a vector of perimeter points,,
    // it also has an integrated signal and maybe some other things..
    std::set<line> lines;
    std::vector<twoDPoint> perimeter;
    std::vector<twoDPoint> smoothPerimeter;  // make a smoothened version as well..
    std::vector<float> inversions;           // gets set by the function.. 
    float totalSignal;
    int min_x, max_x;  
    int min_y, max_y;   // useful parameters to have.. 
    Nucleus(){
	totalSignal = 0;
    }
    void smoothenPerimeter();     // do something to smoothen the perimeter.. 
    void incrementSignal(float* source, int width, int height);    // this is not the most efficient way of doing it. but.. 
    std::vector<float> inversionScores(float* source, int width, int height);   // look for inlets.. 
};

// a voxel that we can draw in a 3 dimensional circumstance..
struct drawVoxel {
    int x, y, z;      // coordinates.
    float r, g, b, a; // the drawing thingy..
    drawVoxel(){
	x = y = z = 0;
	r = g = b = a = 0;
    }
    drawVoxel(int X, int Y, int Z, float R, float G, float B, float A){
	x = X;
	y = Y;
	z = Z;
	r = R;
	g = G;
	b = B;
	a = A;
    }
};

struct voxelVolume {
    int w, h, d;   // width, height and depth..
    float width, height, depth;     // the actual dimensions (needed to set the scales appropriately)..
    std::vector<drawVoxel> voxels;  // the thing to draw..
    voxelVolume(){
	w = h = d = 0;
	width = height = depth = 0;
    }
    voxelVolume(int W, int H, int D, float Width, float Height, float Depth){
	w = W;
	h = H;
	d = D;
	width = Width;
	height = Height;
	depth = Depth;
    }
};

#endif
