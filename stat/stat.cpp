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



// some statistical functions like, hope this works..
#include <qglobal.h>
#include <math.h>
#include <vector>
#include "stat.h"
#include <iostream>
#include <stdlib.h>
#include <string.h> 
#include <algorithm>   


using namespace std;

lin_function fit_weighted_linear(vector<float>& y, vector<float>& x, vector<float>& w){
    //I'm not mathematician, so the following is calculated in some very simple ways..
    double wMeanX, wMeanY;
    double covar, SS;
    lin_function lf;
    if(y.size() != x.size() || y.size() != w.size()){
	cerr << "Stat fit_weighted_linear : vectors are of unequal length can not compute" << endl;
	return(lf);
    }
    wMeanX = wMeanY = 0;
    double N = 0;
    covar = SS = 0;
    for(uint i=0; i < y.size(); ++i){
	wMeanX += x[i] * w[i];
	wMeanY += y[i] * w[i];
	N += w[i];
    }
    wMeanX = wMeanX / N;
    wMeanY = wMeanY / N;
    
//    cout << "wMeanX : " << wMeanX << endl << "wMeanY : " << wMeanY << endl;

    // and then work out ..
    for(uint i=0; i < y.size(); ++i){
	covar += w[i] * ( (x[i] - wMeanX) * (y[i] - wMeanY) );
	SS += w[i] * ( (x[i] - wMeanX) * (x[i] - wMeanX) );
    }
    covar = covar / N;
    SS = SS / N;
//    cout << "coVar : " << covar << "  SS : " << SS << endl;
    double slope = covar / SS;
    double intercept = wMeanY - slope * wMeanX;
    lf.a = intercept;
    lf.b = slope;
    return(lf);
}


bool dist_stats(float& mean, float& var, vector<float>& values, float minValue, float maxValue){
    // take values to represent a distribution.
    // calculate the mean and max values..
    // for the thingy..
    mean = 0; 
    var = 0;
    if(maxValue - minValue <= 0){
	cerr << "dist_stats : maxValue and minVaule are inappropriate : " << minValue << " --> " << maxValue << endl;
	return(false);
    }
    float N = 0;
    float range = maxValue - minValue;
    for(uint i=0; i < values.size(); i++){
	// first calculate the value represented..
	float v = minValue + range * (float(i) + 0.5)/float(values.size());
	// and then simply.. 
	N += values[i];
	mean += (v * values[i]);
    }
    mean = mean / N;      // 
    // and then increment the variance. This is not so pretty but should basically work
    for(uint i=0; i < values.size(); i++){
	float v = minValue + range * float(i)/float(values.size());
	var += values[i] * (mean - v) * (mean - v);   // there are values[i] number of positions with value v
    }
    var = var/N;    // since this may be a real distribution, we just don't know.. 
    return(true);
}
    
float mean(vector<float>& v){
  float sum=0;
  for(int i=0; i < v.size(); i++){
    sum += v[i];
  }
  return(sum/v.size());
}
/*
float std_dev(vector<float>& v){
  float m = mean(v);
  float sqsum =0;
  for(int i=0; i < v.size(); i++){
    sqsum += (v[i]-m)*(v[i]-m);
  }
  return(sqrt(sqsum/(v.size()-1)));
}
*/
float std_dev(vector<float>& v){
  //
  // Note..      SS = sum of squared devieates (squared sum)
  // AND         SS = sum(X^2) - ((sum(X))^2 / N)   --- check the parentheis carefully 
  // so          std = sqrt( SS / N-1)
  // which should be reasonably fast to calculate.. (rather than using the std function.. (which I could rewrite)
  if(v.size() > 1){
    float xsum = 0;
    float xsquaredSum = 0;
    for(int i=0; i < v.size(); i++){
      xsum += v[i];
      xsquaredSum += (v[i]*v[i]);
    }
    float SS = xsquaredSum - (xsum*xsum)/(float)v.size();
    if(SS < 0){
	cerr << "Impossible SS " << SS  << endl;
    }
    //cout << "SS : " << SS << "\tstd : " << sqrt(SS/(float)(v.size()-1)) << endl;
    return(sqrt(SS/(float)(v.size()-1)));
    //    cout << "doing new standard deviation.. " <<sqrt( (xsquaredSum - ((xsum*xsum)/(float)v.size()) ) / (float)v.size()-1)
    // << endl;
    //return(sqrt( (xsquaredSum - ((xsum*xsum)/(float)v.size()) ) / (float)v.size()-1));
  }
  cout << "vector too small returning impossible std deviation of -1 " << endl;
  return(-1);        // hmmm,,,, hmmm
}

bool mean_std(float& mean, float& std_dev, float* v, int l){
    if(l < 2)
	return(false);
    mean = std_dev = 0;
    float sqSum = 0;
    float sum = 0;
    for(int i=0; i < l; ++i){
	sum += v[i];
	sqSum += v[i] * v[i];
    }
    cout << "mean_std sum : " << sum << " and l is : " << l  << " and float(l) is " << float(l) << endl;
    mean = sum / (float)l;
    float SS = sqSum - (sum * sum)/(float)l;
    if(SS < 0){
	cerr << "stat mean_std SS is less than 0. Bummer setting std to 1";
	return(false);
    }
    std_dev = sqrtf(SS/(float)(l - 1));
    return(true);
}

float min(vector<float>& v){
  float minimum;
  if(v.size() < 1){
    cout << "min function from stat.cpp.. -vector has size 0, returning an unintialized float." << endl
	 << "be aware that you should probably run screaming out of the room, and vanquish any" << endl
	 << "ambitions you ever had to get anything done" << endl;
    return(minimum);  // which is actually really bad, because we need to throw an exception...
  }
  minimum = v[0];
  for(int i=1; i < v.size(); i++){
    if(minimum > v[i]) { minimum = v[i]; }
  }
  return(minimum);
}

float max(vector<float>& v){
  float maximum;
  if(v.size() < 1){
    cout << "max function from stat.cpp.. -vector has size 0, returning an unintialized float." << endl
	 << "be aware that you should probably run screaming out of the room, and vanquish any" << endl
	 << "ambitions you ever had to get anything done" << endl;
    return(maximum);  // which is actually really bad, because we need to throw an exception...
  }  
  maximum = v[0];
  for(int i=1; i < v.size(); i++){
    if(maximum < v[i]) { maximum = v[i]; }
  }
  return(maximum);
}

void range(float* f, float& min, float& max, unsigned int l){
  if(!l){
    min = max = 0;
    return;
  }
  min = max = f[0];
  for(uint i=0; i < l; ++i){
    min = min > f[i] ? min : f[i];
    max = max < f[i] ? max : f[i];
  }
  return;
}

float med(vector<float> v){
  float med = 0;
  if(!v.size()){
    return(med);
  }
  if(v.size() == 1){
    return(v[0]);
  }
  // otherwise we'll need to sort the vector..
  sort(v.begin(), v.end());
  if(v.size() % 2){     // i.e. we have an odd number
    return(v[v.size()/2]);
  }
  return( (v[v.size()/2] + v[v.size()/2 - 1])/2);
}

float mode_average(float* f, unsigned int l, unsigned int d, float min, float max){
  if(max == min || min > max)
    range(f, min, max, l);
  if(!l || (min == max))
    return(min);
  float r = max - min;
  if(!d)
    d = 100;
  float fd = (float)(d-1);
  float* counts = new float[d];
  memset((void*)counts, 0, sizeof(float) * d);
  // the use of roundf in this code causes the positions
  // to act like centroids. The first and last positions
  // will be undercounted, but there is not much I can do
  // about that.
  int out_of_range = 0;
  int nan_count = 0;
  for(uint i=0; i < l; ++i){
    if(f[i] > max || f[i] < min){
      out_of_range++;
      continue;
    }
    if(f[i] != f[i])
      nan_count++;
    counts[ (int)roundf((fd * (f[i] - min)) / r) ]++;
  }
  int max_i = 0;
  for(int i=0; i < d; ++i){
    max_i = counts[i] > counts[max_i] ? i : max_i;
  }
  cout << endl;
  // and then convert max_i to a value.
  return( min + r * (float(max_i) / fd) );
}

float euclidean(vector<float>& v1, vector<float>& v2){
  // simply calculate the euclidean distance between two things.
  float d = 0;     // the distance..
  if(v1.size() != v2.size()){
    cout << "stat.cpp, euclidean function. vectors of different sizes. returning -1" << endl;
    d = -1;
    return(d);
  }
  for(int i=0; i < v1.size(); i++){
    d += ((v1[i]-v2[i])*(v1[i]-v2[i]));
    //    d += pow(v1[i]-v2[i], 2);  // don't call pow, its slow.. 
  }
  d = sqrt(d);
  return(d/v1.size());
}

float sqEuclidean(vector<float>& v1, vector<float>& v2){
  float d = 0;     // the distance..
  if(v1.size() != v2.size()){
    cout << "stat.cpp, euclidean function. vectors of different sizes. returning -1" << endl;
    d = -1;
    return(d);
  }
  for(int i=0; i < v1.size(); i++){
    d += ((v1[i]-v2[i])*(v1[i]-v2[i]));
   }
  return(d);
}
  

vector<float> z_score(vector<float>& v){
  vector<float> z(v.size());
  float m = mean(v);
  float s = std_dev(v);
  for(int i=0; i < z.size(); i++){
    z[i] = (v[i]-m)/s;
  }
  return(z);
}

void zScore(vector<float>& v){
  ///// MODIFIES THE COPY IT RECEIVES AS THIS IS FASTER..
  float m = mean(v);
  float s = std_dev(v);
  for(int i=0; i < v.size(); i++){
    v[i] = (v[i]-m)/s;
  }
}

void zThreshold(vector<vector<float> >& v){
  ///////////  NOTE this is a horribly INEFFICIENT way of writing this function.. if it's useful rewrite it later..
  // does a normal z-score normalisation, then calculates three more vectors containing..
  //  1. the mean values across the experiments..
  //  2. the (mean - min)/std for at each point.. (as a measure of the actual thingy..)..
  //  3. a kind of thresholding function..  -- check the source code for the description..
  if(!v.size()){
    return;
  }
  // first normalise each individual vector.. 
  for(int i=0; i < v.size(); i++){
    zScore(v[i]);
  }
  // get some mins and medians..
  vector<float> means(v[0].size(), 0);
  vector<float> stds(v[0].size(), 0);
  vector<float> thr(v[0].size(), 0);     // the result of the thresholding operation.
  for(int i=0; i < v.size(); i++){
    for(int j=0; j < v[0].size(); j++){
      means[j] += v[i][j];
      stds[j] += (v[i][j] * v[i][j]);
    }
  }
  if(!means.size()){
    return;
  }
  float minMean = means[0]/v.size();
  for(int i=0; i < means.size(); i++){
    stds[i] = sqrt(  (stds[i] - ((means[i] * means[i])/v.size()))/(v.size() -1) );   // whoaaa.a.a.a.a.a.
    means[i] = means[i]/v.size();
    if(means[i] < minMean){ minMean = means[i]; }
  }
  /// and now we can go through and change everything.. 
  /// try a fifth order hill equation to give a sigmoidal shape.. i.e. ignore the very log things
  /// and blabllslslsl 

  float sigma = 0.75; // - minMean;    // thresholding function..
  float order = 5;
  // -- put the mean at the end of v..
  v.push_back(means);     // as we will modify below..
  for(int i=0; i < means.size(); i++){
    means[i] = (means[i] - minMean);
    //means[i] = (means[i] - minMean)/stds[i];
    thr[i] = pow(means[i]/sigma, order)/(1 + pow(means[i]/sigma, order)); 
    //thr[i] = M_E - exp(-(means[i] - minMean)/sigma);  // logarithmic peaking at value of e.. ..and lowest value at 1.. hmm, 
  }
  v.push_back(means);
  v.push_back(thr);
}
      

/*
float median(vector<float> v){
  // not particularly efficient, but what the hell
  if(v.size() < 1){
    return(0);
  }
  sort(v.begin(), v.end());
  int mid = v.size()/2;
  float median;
  if(v.size() %2 == 0){
    median = (v[mid] + v[mid-1])/2;
  }else{
    median = v[mid];
  }
  return(median);
}

float mad(vector<float>& v){
  // first get median..
  float m = median(v);
  // then go through each thingy and get the 
  // the difference
  vector<float> absdev(v.size());
  for(int i=0; i < v.size(); i++){
    absdev[i] = fabs(v[i] - m);
  }
  // then get the median of the deviations..
  float m_ad = median(absdev);
  return(m_ad);
}
  
vector<float> m_score(vector<float>& v){
  vector<float> ms(v.size());
  float m = median(v);
  float m_ad = mad(v);
  for(int i=0; i < ms.size(); i++){
    ms[i] = 0.6745 * ((v[i] - m)/m_ad);
  }
  return(ms);
}
*/
vector<float> d_series(vector<float>& v){
  // a somewhat confused concept, but one that may have some useful parts to it
  // essentially returns a vector of v.size(-1) which represents the difference
  // between the first member of v, and the rest.. -may be useful in the case where
  // the first member is some sort of control, -then the resulting point in N-dimensional
  // space will tend to represent how different that 'gene' is from its control state..
  // or the reproducibility.. or something... -useful for 3 samples as well, as it allows 
  // a 2 d representation of the relationships.. in thingss...
  vector<float> d(v.size()-1);
  for(int i=1; i < v.size(); i++){
    d[i-1] = v[i]-v[0];
  }
  return(d);
}


vector<int> f_distribution(vector<float>& v, float minV, float maxV, int divs){
  // choice: 0 -all, 1, pm, 2 is mm;
  vector<int> dis(divs, 0);     // i.e. make a vector with divs members..
  // go through the vector members and initialise to 0. this may not be 
  // necessary, but I prefer to do it nevertheless.
  //for(int i=0; i < dis.size(); i++) { dis[i] = 0; }
  // count ..
  for(int i=0; i < v.size(); i++){
    if(v[i] >= minV && v[i] <= maxV){ 
      dis[(int)(((v[i]-minV)*(float)divs)/(maxV-minV))]++;
    }
  }
  return(dis);
}

vector<int> l_distribution(vector<float>& v, float minV, float maxV, int divs){
  // double log distribution,, not sure how to do this, but lets have a go.. 
  vector<int> dis(divs, 0);
  
  if(minV <= 0){
    cout << "returning a load of 0's as the minValue is less than 0 " << endl;
    return(dis);
  }
  float range = log(maxV)-log(minV);
  float lminV = log(minV);
  int index; 
  for(int i=0; i < v.size(); i++){
    if(v[i] >= minV && v[i] <= maxV){ 
      index = (int)(((log(v[i])-lminV)/range)*(float)divs);
      if(index > 0 && index < dis.size()){
	dis[index]++;
      }else{
	cout << "index out of range, index: " << index << "\tsize: " << dis.size() << endl;
	cout << "v[" << i << "] : " << v[i] << "\tminV: " << minV << "\trange: " << range << endl;
      }
      //      dis[(int)((log(v[i]-minV)/(range))*(float)divs)]++;
      //      dis[(int)log(((v[i]-minV)*(float)divs)/(maxV-minV))]++;
    }
  }
  return(dis);
} 
/*
vector<float> norm_median(vector<float> v){
  // calculate the median value, and divide all members by it
  // simple..
  // operates on a local copy that gets modified..
  if(v.size() == 0) { return v; }
  sort(v.begin(), v.end());
  int mid = v.size()/2;
  float median;
  if(v.size() % 2 == 0){
    median = (v[mid] + v[mid-1])/2;
  }else{
    median = v[mid];
  }
  for(int i=0; i < v.size(); i++){
    v[i] = v[i]/median;
  }
  return v;
}
*/
vector<float> norm_mean(vector<float> v){
  // just divides by the mean value..
  float sum = 0;
  for(int i=0; i < v.size(); i++) { sum += v[i]; }
  sum = sum/(float)v.size();  // convert to mean..
  for(int i=0; i < v.size(); i++) { v[i] = v[i]/sum; }
  return(v);
}

/*
vector<float> norm_min_median(vector<float> v){
  if(v.size() == 0) { return(v); }
  if(v.size() == 1) {
    v[0] = 0;
    return(v);
  }
  sort(v.begin(), v.end());
  float median;
  float min = v[0];
  int mid = v.size()/2;
  if(v.size() % 2 == 0){
    median = (v[mid] + v[mid-1])/2;
  }else{
    median = v[mid];
  }
  // go through and normalise
  float min_med = median-min;
  for(int i=0; i < v.size(); i++){
    v[i] = (v[i]-min) / min_med;
  }
  return(v);
}
*/

vector<float> norm_min_mean(vector<float> v){
  // find the min, and mean values..
  float mean = 0;
  float min = v[0];
  for(int i=0; i < v.size(); i++){
    mean += v[i];
    if(v[i] < min) { min = v[i]; }
  }
  mean = mean/v.size();
  float min_mean = mean-min;
  // go through and normalise
  for(int i=0; i < v.size(); i++){
    v[i] = (v[i]-min) / min_mean;
  }
  return(v);
}

float maxMeanDeviation(vector<float>& v){
  // returns the maximum deviation from the mean value in terms of abs((value-mean)/mean);
  float meanD = mean(v);
  float maxD = 0;
  for(int i=0; i < v.size(); i++){
    if( fabs((v[i] - meanD)/meanD) > maxD){
      maxD = fabs((v[i] - meanD)/meanD);
    }
  }
  return(maxD);
}

float binomialProb(int N, int s, float p){
  // calculates the binomial probability for seeing s number of successes
  // from N trials where ps is the probability of seeing a success
  // 
  //
  //   The binomial equation is..

  // 
  //                N!        
  //     P(s) =  --------- * (p)^s * (1-p)^(N-s)
  //             s! (N-s)!
  // 
  // which ofcourse becomes simpler if the probability of success and failure are the same,
  // but we can't really assume that. Never mind. 
  // factorials get tricky if they are large so we should probably reduce the equation in a managable manner
  // which shouldn't be too difficult, just follow my procedure in Hypgeo.cpp.. (~/cpp/stat/)
  // 
  // First work out the left hand ratio, using two vectors of ints..
  float ratio = 1;  // top over num 
  if(s != N){
    for(int i=1; i <= (N-s); i++){
      //cout << "multiplying ratio by : " << s+i << " / " << i << "\t";
      ratio = ratio * ((float)(s+i) / (float)i);
      //      ratio = ratio * ((float)(N-i) / (float)(s)); 
      // cout << "ratio: " << ratio << endl;
    }
  }
  // and now we have the ratio its very easy to do the calculation using the power function..
  return(ratio *  pow(p, s) * pow(1-p, N-s));
}
    
float absolute(float v){
  return(v > 0 ? v : -v);
}

float modelNormalise(vector< vector<float> >& v){
  float error = 0;     // the sum of the errors or something like that.. doesn't matter too much right now.. 
  if(v.size() < 2){
    return(error);
  }
  for(int i=0; i < v.size(); i++){
    if(v[i].size() != v[0].size()){
      return(error);
    }
  }
  vector<float> std_devs(v.size());
  vector<float> means(v.size());
  vector<float> var_coeffs;     // one for each probe set.. --
  var_coeffs.reserve(v.size());
  for(int i=0; i < v.size(); i++){
    std_devs[i] = std_dev(v[i]);
    means[i] = mean(v[i]);
    if(means[i] > 0){
      var_coeffs.push_back(std_devs[i]/means[i]);     // just hope that there are no 0's.. 
    }
  }
  //// note, that although negative means will end up with what are theoretically crazy numbers,,, -- 
  ///  but since we are going to take a median value of these guys this won't matter if half of the close to 0 values
  ///  are negative and the others are positive.. -- in other words, the numbers will just even out.. so it's ok..
  
  /// note that whereas the above is certainly true,, -- we don't want to end up subtracting from probe pairs that alreeady have a 
  /// negative mean.. -and the simplest way around that is just to do an absoulute on the above. Anyway, we don't yet know how it will
  /// end up looking.. so.. 
  
  if(var_coeffs.size() < 1){
    return(error);
  }
  // get the median var_coeffs..
  float med_var_coeff = med(var_coeffs);   /// hmmmm... 
  
  // calculate a set of addition factors that convert the means of all the things to a value that changes the var coefficient to
  // the median.. -- i.e. set everything to have the same variation coefficient..
  vector<float> addFactors(v.size());
  for(int i=0; i < v.size(); i++){
    addFactors[i] = (std_devs[i]/med_var_coeff) - means[i];    // think that is ok..
  }
  
  // add these factors all of the values for each given probe,, -- this should set all of the probes to have the 
  // the same standard devitation / mean.. -- 
  // 
  // - we now have a fixed relationship between the mean and the std deviation for all of them. --- I would suggest dividing everything
  // - by their own std deviation, though we could just as well divide by their means.. -it would make very little difference..
  
  // just go through and normalise all values..
  for(int i=0; i < v.size(); i++){
    for(int j=0; j < v[i].size(); j++){
      v[i][j] = (v[i][j] + addFactors[i])/std_devs[i];
    }
  }
  // ahh, I'm suppposed to return some informative value.. hmm. oh never mind maybe later..
  error = 1;
  return(error);    
}

float model2Normalise(vector< vector<float> >& v){
  float error = 0;
  if(v.size() < 2){
    return(error);
  }
  for(int i=0; i < v.size(); i++){
    if(v[i].size() != v[0].size()){
      return(error);
    }
  }
  // ok..
  // determine the std_devs and the means for each vector..
  vector<float> std_devs(v.size());
  vector<float> means(v.size());
  for(int i=0; i < v.size(); i++){
    std_devs[i] = std_dev(v[i]);
    means[i] = mean(v[i]);
  }
  /// now find the median std dev..
  float med_std_dev = med(std_devs);
  float med_means = med(means);
  vector<float> factors(v.size());
  // and calculate some mul_factors that set the std_dev to the same for each thingy..
  for(int i=0; i < v.size(); i++){
    factors[i] = med_std_dev / std_devs[i];    /// hmmm, maybe this is not perfect, as the std_dev is the sum of the squares of the deviations, but let's try.
  }
  // transform by multiplying by the factor, and then setting all means to 0.. 
  // hmm, this sounds a bit dodgy..
  for(int i=0; i < v.size(); i++){
    for(int j=0; j < v[i].size(); j++){
      v[i][j] = (v[i][j] * factors[i]) + (med_means - means[i] * factors[i]);
    }
  }
  error = 1.0;
  return(error);
}
				 
vector<float> devsFromMean(vector<vector<float> > v){
  // find the mean profile, then determine the distance of each probe from the mean profile, then calculate the number
  // of standard deviations this is from the mean distance from the mean profile..  ok..
  // then we can use this data struct to remove outliers and stuff. Sounds like this could be written much more 
  // nicely, but I'm in a bit of a hurry at the moment and I just want the function to look at ..
  vector<float> meanDeltas(v.size(), 0);
  float sigma = 0;                     // the standard deviation in k-dimensional space or something.. 
  if(v.size() < 2){
    return(meanDeltas);
  }
  vector<float> meanProfile(v[0].size(), 0);   // all the probe pairs need to have the same thing,, we are not going to check, but it's important
  for(int i=0; i < v.size(); i++){
    if(v[0].size() != v[i].size()){
      cerr << "stat.cpp devsFromMean, the bloody component vectors are of different size.. " << endl;
      return(meanDeltas);
    }
    zScore(v[i]);          // modifies the vector, and should perhaps be useful.. 
    for(int j=0; j < v[i].size(); j++){
      meanProfile[j] += v[i][j]/(float)v.size();
    }
  }
  zScore(meanProfile);   // again, we are only interested in the pattern of change.. 
  // so we have the means.. now we just go through and calculate the deltas,,
  float n = (float)(v.size()-1);
  for(int i=0; i < v.size(); i++){
    float sqEuclid = sqEuclidean(v[i], meanProfile);   // the squared euclidean distance..
    //cout << "sqEuclid is " << sqEuclid << "\t";
    meanDeltas[i] = sqrt(sqEuclid);;    // think I don't need this, but need a gradual rewrite.. 
    //cout << "   and delta is " << meanDeltas[i] << endl;
    sigma += (sqEuclid/n);
  }
  //  cout << "and the square of sigma is " << sigma << endl;
  sigma = sqrt(sigma);   // Ok.. 
  // oh lalala... and lets get the mean, and std deviation..
  //float mv = mean(meanDeltas);   // ---- actually we don't really need this at all.. 
  //float std = std_dev(meanDeltas);   // which ofcourse is a very slow way of doing this operation, fix this sometime..
  // and devide by the std to get some sort of useful value..
  //   if(!std){
  //     cerr << "stat.cpp some strange function.. std is 0 " << endl;
  //     return(meanDeltas);
  //   }
  for(int i=0; i < meanDeltas.size(); i++){
    meanDeltas[i] = (meanDeltas[i])/sigma;   
  }
  return(meanDeltas);
}

char* randomBytes(int length){
  // assumes that srand has been called somewhere else in the program.. -I'm assuming that this will carry through..
  cout << " generating : randomBytes" << endl;
  char* bytes = new char[length];
  for(int i=0; i < length; i++){
    bytes[i] = (char)(rand() % 256);      // the new rand function should give equally random numbers in the low and high bits so this should be ok..
    //cout << i << "  " << bytes[i] << endl; 
  }
  //cout << "randomBytes made a rand array " << bytes << endl;   // which will probably confuse the console..
  // -I can't do cout, as this is not a 0 terminated string.. but truly a collection of 
  return(bytes);
}


int mutateBytes(char* bytes, int length, float freq){
  // freq should be a value between 0 and 1... -where 1 is mutate all bytes and 0 none.. 
  int mutateCount = 0;
  for(int i=0; i < length; i++){
    float decider = (float)rand()/ (float)RAND_MAX;  // which should give me a value between 0 and 1..
    if(freq < decider){                               // NOT PERFECT AS IF decider is 1 then we won't mutate even if freq is 1. but sort of ok.
      //cout << "mutating " << bytes[i] << "  to : ";
      bytes[i] = (char)(rand() % 256);
      //cout << bytes[i] << endl;
      mutateCount++;
    }
  }
  return(mutateCount);
}

int softMutateBytes(char* bytes, int length, int maxDistance, float freq){
  // frequence as above for mutatBytes..
  // maxDistance is the maximum difference in numerical value that the char can change by
  // 
  int mutateCount = 0;
  for(int i=0; i < length; i++){
    float decider  = (float)rand()/ (float)RAND_MAX;
    //cout << "frequency : " << freq << "  decider : " << decider << endl;
    //cout << "bytes " << bytes[i];
    if(freq < decider){
      int delta = maxDistance - (rand() % (1+maxDistance*2));    // value between -maxDistance to +maxDistance..
      uint newValue = (int)bytes[i] + delta;
      if(newValue > 255){ newValue = 255; }
      bytes[i] = (char)newValue;                               // what happens at 0 and more than 256 ,, I don't konw..
      ///// clumsy, -- I'm sure there's a better way.. basic c stuff,, that i never learnt..
      mutateCount++;
    }
    // cout << "  -> " << bytes[i] << endl;
  }
  return(mutateCount);
}

void expandPositions(char* oldBytes, char* newBytes, int oldLength, int newLength){
  // simualate expansion.. of single sites,, -- newBytes gains some insertions at randomly picked locations..
  // these insertions are in fact replications of the site before,, so AAAG could change to AAAGG or something like that..
  if(newLength < oldLength){
    // this will probably at some point create a crash..
    // anyway, try to copy oldBytes into newBytes.. without causing too much trouble..
    for(int i=0; i < newLength; i++){
      newBytes[i] = oldBytes[i];
    }
    return;
  }
  // create a vector of position determiners.. --- doesn't work if we hit the same place twice..    
  vector<int> positions(oldLength, 0);
  for(int i=0; i < (newLength-oldLength); i++){
    int pos = rand() % positions.size();
    positions[pos]++;
  }
  // and finally assign the values in the newBytes array.. hmm.
  int j = 0;    // counter for new array positions.. 
  for(int i=0; i < oldLength; i++){
    //cout << "\t\t" << oldBytes[i];
    newBytes[j++] = oldBytes[i];
    //cout << "\t" << newBytes[j-1];
    while(positions[i]){ // keep going until the value is 0.  ho yeaahh.. 
      newBytes[j++] = oldBytes[i];
      //cout << "\t" << newBytes[j-1];
      positions[i]--;        
    }
    //cout << endl;
  }
  // and now we are done, if the arithmetic is wrong, we may crash horribly, but what the hell.. it's ok.. 
}
