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

#include "imageFunc.h"
#include <iostream>
#include <stdio.h>

using namespace std;

offsets ImageFunc::findOffsets(float* a, float* b, unsigned int w, unsigned int h){
    int win = windowSize(w, h);
    return(findOffsets(a, b, w, h, win));
}

offsets ImageFunc::findOffsets(float* a, float* b, unsigned int w, unsigned int h, int window){
    // first check that window is ok.
    offsets off;  // so we can return a void one..
    if(!window){
	cerr << "ImageFunc : window size is 0 no reasonable comparison can be made" << endl;
	return(off);
    }
    if(window > windowSize(w, h)){
	cerr << "ImageFunct::findOffsets window size too large : " << window << "  corrected to ";
	window = windowSize(w, h);
	cerr << window << endl;
    }
    if(!w || !h){
	cerr << "ImageFunc::findOffsets nonsensical window size " << w << ", " << h  << endl;
	return(off);
    }
    // and then do the comparisons. In order for the maths to come out appropriately the comparisons have 
    // normalised individually
    // for which we need some new buffers..
    float* a_buffer = new float[w * h];
    float* b_buffer = new float[w * h];
    unsigned int edge_length = 2 * window + 1;
    unsigned int corr_n = (2 * window + 1) * (2 * window + 1);
    float* corr_values = new float[corr_n];   // since we want to normalise the values.. 
    for(int dx=-window; dx <= window; dx++){
	//cout << "dx : " << dx;

//	printf("%-2.0d  ", dx);

	int bx = 0;
	int ex = w;
	if(dx < 0){ bx = -dx; }
	if(dx > 0){ ex = w - dx; }
	for(int dy=-window; dy <= window; dy++){
	    int by = 0;
	    int ey = h;
	    if(dy < 0){ by = - dy; }
	    if(dy > 0){ ey = h - dy; }
	    // and then make the temporary storage..
	    unsigned int l = 0;
	    for(int yp=by; yp < ey; yp++){
		for(int xp=bx; xp < ex; xp++){
		    b_buffer[l] = b[yp * w + xp];
		    a_buffer[l] = a[(yp + dy) * w + xp + dx];
		    ++l;
		}
	    }
	    float corr = correlate(a_buffer, b_buffer, l);
	    corr_values[(window + dy) * edge_length + window + dx] = corr;
//	    printf(" % .3f  ", corr);
	    if(corr > off.corr){
		off.corr = corr;
		off.dx = dx;
		off.dy = dy;
	    }
	}
//	cout << endl;
    }
    normalise(corr_values, corr_n);
    float max_corr = maxValue(corr_values, corr_n);
    off.norm_corr = max_corr;
    // I want to print out the corrs as well.. 
 
//    for(int dx=-window; dx <= window; dx++){
// 	printf("%-2.0d  ", dx);
// 	for(int dy=-window; dy <= window; dy++){
// 	    printf(" % .3f  ", corr_values[(window + dy) * edge_length + window + dx]);
// 	}
// 	cout << endl;
//     }

    delete corr_values;
    delete a_buffer;
    delete b_buffer;
    return(off);
}

unsigned int ImageFunc::windowSize(unsigned int w, unsigned int h){
    // actually we can just say,, window = md - min_overlap..
    int min_overlap = 4;

    int md = w < h ? w : h; 
    // min dimension..
    int win = md - min_overlap;
    if(win < 0){
	return(0);
    }
    return((unsigned int)win);
//    unsigned int win = (md % 2) ? md/2 : (md/2) - 1;    // if md is odd then window = md / 2
//    return(win);
}

float ImageFunc::correlate(float* a, float* b, unsigned int l){
    normalise(a, l);
    normalise(b, l);
    float score = 0;
    for(uint i=0; i < l; i++){
	score += a[i] * b[i];
    }
    score = score / float(l);
    return(score);
}

void ImageFunc::normalise(float* values, unsigned int l){
    // assume values contains the approriate things..
    float Sq = 0;  // squared sum
    float S = 0;   // sum
    for(uint i=0; i < l; i++){
	Sq += (values[i] * values[i]);
	S += values[i];
    }
    float n = (float)l;
    float std =  sqrt( (Sq - S*S/n)/(n - 1.0) );
    float mean = S / n;
    // and then go through an normalise the values..
    for(uint i=0; i < l; i++){
	values[i] = (values[i] - mean) / std;
    }
    
}

float ImageFunc::maxValue(float* values, unsigned int l){
    if(!l){
	return(0.0);
    }
    float max = values[0];
    for(uint i=1; i < l; i++){
	if(max < values[i]){ max = values[i]; }
    }
    return(max);
}
