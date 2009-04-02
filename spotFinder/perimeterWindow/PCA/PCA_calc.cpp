#include "PCA_calc.h"
#include <string.h>

void z_score(float* v, int l){
    float mean, var;
    mean = var = 0;
    for(uint i=0; i < l; ++i){
	mean +=  v[i];
    }
    mean /= (float)l;
    for(int i=0; i < l; ++i){
	var += (mean - v[i])*(mean - v[i]);
    }
    var /= (float)(l-1);
    var = sqrtf(var);
    // this is the ugly way of calculating standard deviations, but I think
    // it is less sensitive to rounding errors than the (squared sum) - (sum of squares) method.
    // and then divide by std deviation .
    for(int i=0; i < l; ++i){
	l[i] = (l[i] - mean) / var;
    }
    // and return.. 
}

void scale(float* v, int l){
    // make sure all numbers are 0 or more..
    float min, max;
    min = 0;
    max = v[0];
    for(int i=1; i < l; ++i){
	if(v[i] > max)
	    max = v[i];
	if(v[i] < min)
	    min = v[i];
    }
    for(int i=0; i < l; ++i)
	v[i] = (v[i] - min) / max;
}

PCA_calc::PCA_calc(float* data, int objects, int dimensions){
    o_data = data;
    objNo = objects;
    dimNo = dimensions;

    initialize();  // this actually does everything.. 
}

void PCA_calc::initialize(){
    // first create the transformed copy.
    // and them memcpy it before transforming it.
    t_data = new float[objNo * dimNo];
    memcpy((void*)t_data, o_data, objNo * dimNo * sizeof(float));
    // and then transform individual lines
    for(int i=0; i < objNo; ++i){
	z_score(t_data + i * dimNo, dimNo);
    }
    
    // then we need to find the first line....
    // but first initialize the two matrices holding the information that we are gathering..
    lines = new float[dimNo * dimNo];
    memSet((void*)lines, 0, dimNo * dimNo * sizeof(float));
    linePositions = new float[objNo * dimNo];  // 
    
    findFirstLine();
    for(int i=1; i < dimNo; ++i){
	findLine(i);
    }
    // those two functions will take care of everything. Maybe lots of calculations, but it depends maybe.. 
}

void PCA_calc::findFirstLine(){
    // for each object, find the direction vector, and add that to the first row (0th) of 
    // the lines matrix. Afterwards we can either divide by the number of members
    // or not bother. 

    // Note that in order to get direction vectors, since the sum is 0, we have to make sure that they point
    // in the same direction. The easiest way is to just set the initial dimension to have a positive sign..
    // and multipy everything by 1 or -1 to make sure that is the case.

    float m;
    float source;  // the data source then we can use pointer arithmetic.. 
    for(int i=0; i < objNo; ++i){
	source = t_data + i * dimNo;
	m = *source > 0.0 ? 1.0 : -1.0;
