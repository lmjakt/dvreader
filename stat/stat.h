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


#ifndef STAT_H
#define STAT_H

#include <vector>


struct weightedValueSet {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> w;
};

struct lin_function {
    // the parameters in the function //
    // y = a + bx
    double a;
    double b;
    lin_function(){
	a = b = 0;
    }
    lin_function(double A, double B){
	a = A;
	b = B;
    }
};

// does a linear regression on the values weighted by the w factors
// (the w gives the thingy.. )
// (weights here are taken as a simple multiplier for a given point, i.e. with a weight of 2, the point
// is counted as having occured 2 times in all the calculations). (I'm not sure if this is appropriate
// but let's see how it goes.. )
lin_function fit_weighted_linear(std::vector<float>& y, std::vector<float>& x, std::vector<float>& w);



// I always wonder about that bloody namespace std thingy..
// as I read somewhere that it was bad, but I can't quite 
// remember where..
float absolute(float v);
float mean(std::vector<float>& v);
float std_dev(std::vector<float>& v);
bool mean_std(float& mean, float& std_dev, float* v, int l);
float min(std::vector<float>& v);
float max(std::vector<float>& v);
void range(float* f, float& min, float& max, unsigned int l);

float med(std::vector<float> v);   // return the median.. use slow sorting function.. -- don't use a reference as we'll sort the vector.. 
float mode_average(float* f, unsigned int l, unsigned int d, float min=0, float max=0);

// a function for calculating variance and mean from a distribution of values (or some such thing that seems similar).
bool dist_stats(float& mean, float& var, std::vector<float>& values, float minValue, float maxValue);

float euclidean(std::vector<float>& v1, std::vector<float>& v2);
float sqEuclidean(std::vector<float>& v1, std::vector<float>& v2);   // return the square of the euclidean, useful for somethings.. 
std::vector<float> z_score(std::vector<float>& v);
void zScore(std::vector<float>& v);                /// MODIFIES VECTOR V !!!!!
void zThreshold(std::vector<std::vector<float> >& v);            // complicated story.. 
float maxMeanDeviation(std::vector<float>& v);        // returns the maximum deviation from the mean value calculated as abs((value-mean)/mean)

std::vector<float> devsFromMean(std::vector<std::vector<float> > v);   // see code for details.. 

//float median(vector<float> v);
//float mad(vector<float>& v);
//vector<float> m_score(vector<float>& v);

std::vector<float> d_series(std::vector<float>& v);  // see stat.cpp for description of this vague concept

std::vector<int> f_distribution(std::vector<float>& v, float minV, float maxV, int divs);
std::vector<int> l_distribution(std::vector<float>& v, float minV, float maxV, int divs);
//vector<float> norm_median(vector<float> v);
std::vector<float> norm_mean(std::vector<float> v);   // these operate on and change a local copy, hence not passed by reference
//vector<float> norm_min_median(vector<float> v); // value-min / median-min
std::vector<float> norm_min_mean(std::vector<float> v);   // value-min / mean-min.

float modelNormalise(std::vector<std::vector<float> >& v);  // normalise assuming an extension of the Li and Wong model..
                                                   // essentially set the std / mean to the same for all of the things by 
                                                   // changing the mean.. -- set to the median value of the coordinates..
                                                   // then directly calculate sensitivity parameters based on the actual std_deviations
                                                   // or maybe the means.. -- I'm not sure at the moment, just trying things.. first.
float model2Normalise(std::vector< std::vector<float> >& v); // a bit different.. 
//float model2Normalise(vector< vector<float> >& v); -- why repeated ??? 

float binomialProb(int N, int s, float p);

//////////// some string or char* functions..
char* randomBytes(int length);             // uses new to allocate memory, so remember to delete after use !!.. 
int mutateBytes(char* bytes, int length, float freq); // mutate to random the bytes in  (freq should be between 0 and 1, but it isn't checked).
int softMutateBytes(char* bytes, int length, int maxDistance, float freq);   // mutate with a set maxDistance.. 
void expandPositions(char* oldBytes, char* newBytes, int oldLength, int newLength); // simulate nucleotide expansion.. 

#endif




