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

#include "objectComparer.h"
#include <string.h>
#include <math.h>
#include <iostream>

using namespace std;

void ObjectComparer::setData(float* data, int objects, int dimensions, float sigma, float power){
    isFlat = (sigma > 0 && power > 0);
    objNo = objects;
    dimNo = dimensions;

    sig = sigma;
    order = power;
    
    //and then set up the appropriate matrices..
    o_data = data;
    t_data = new float[objNo * dimNo];
    d_data = new float[objNo * objNo];  // which strictly speaking is uneccessary..
    memset((void*)d_data, 0, objNo * objNo * sizeof(float));
    
    cout << "calling normalise" << endl;
    // then just call the appropriate functions..
    normalise();
    cout << "normalise returned.. " << endl;
    if(isFlat)
	flatten();
    cout << "calling compare.. " << endl;
    compare();
    cout << "compare returned" << endl;
    // and we are done..   // we might want to do some of these in a separate loop.. 
}

void ObjectComparer::normalise(){
    // just use a simple z-score normalisation for the data. Note that the data is normalised
    // in a parameterwise manner. i.e. all the parameters for a given parameter are normalised
    // across the objects..
    
    float std, mean; // we need to calculate one of these for each sample
    // use the old way of doing things..
    // as I think we get less rounding errors this way..
    
    // note that objects are in rows and parameters are in columns so.. 

    // so for each parameter.. (a parameter is a dimension.. bad naming call)
    for(int p=0; p < dimNo; ++p){
	std = mean = 0;
	for(int o=0; o < objNo; ++o)
	    mean += o_data[o * dimNo + p];
	for(int o=0; o < objNo; ++o)
	    std += (o_data[o * dimNo + p] - mean) * (o_data[o * dimNo + p] - mean);
	std /= (float)dimNo;
	std = sqrtf(std);
	for(int o=0; o < objNo; ++o)
	    t_data[o * dimNo + p] = (o_data[o * dimNo + p] - mean) / std;  // potential divide by 0
    }
    // and that's all folks..
}

void ObjectComparer::flatten(){
    // since this only works with 0 or larger values we have to find the min value for
    // each object and use that. 
    // That's bad, if we have outliers, as these will skew everything else.. It would perhaps be
    // better to do something a bit smarter, but what the hell..
    
    // for each parameter..
    for(int p=0; p < dimNo; ++p){
	float min = t_data[p];
	for(int o=1; o < objNo; ++o)
	    min = t_data[o * dimNo + p] < min ? t_data[o * dimNo + p] : min;
	for(int o=1; o < objNo; ++o){
	    float* pt = t_data + o * dimNo + p;
	    (*pt) -= min;
	    (*pt) = pow((*pt)/sig, order) / (1 + pow((*pt)/sig, order));
	}
    }
    
}

void ObjectComparer::compare(){
    // the d_data matrix should have been initialised to 0, so we can just fill in the non-diagonal parts.
    for(int i=0; i < objNo; ++i){
	cout << "object " << i << endl;
	for(int j=i; j < objNo; ++j){
	    cout << "*";
	    float* d1 = d_data + i * objNo + j;
	    float* d2 = d_data + j * objNo + i;
	    float d = 0;
	    for(int k=0; k < dimNo; ++k){
		cout << ".";
		d += (t_data[i * dimNo + k] - t_data[j * dimNo + k]) * (t_data[i * dimNo + k] - t_data[j * dimNo + k]);
	    }
	    cout << endl;
	    d = sqrtf(d);
	    (*d1) = d;
	    (*d2) = d;
	}
    }
    // and voila.. there we go..
}

objectDistanceInfo ObjectComparer::distances(){
    if(!d_data){
	objectDistanceInfo od;
	return(od);
    }
    vector<int> objects;
    vector<vector<float> > distances;
    distances.resize(objNo);
    for(int i=0; i < objNo; ++i){
	distances[i].resize(objNo);
	objects.push_back(i);
	for(int j=0; j < objNo; ++j){
	    distances[i][j] = d_data[i * objNo + j];
	}
    }
    objectDistanceInfo distInfo(objects, distances, isFlat);
    return(distInfo);
}
