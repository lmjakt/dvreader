#include "nd_classifier.h"
#include <iostream>
#include <math.h>
#include <set>
#include <string.h>

using namespace std;

ND_Classifier::ND_Classifier()
{
  dims = t_size = 0;
  trainData = 0;
}

ND_Classifier::~ND_Classifier()
{
  delete trainData;
}

bool ND_Classifier::setTrainingSet(vector<int> classes, float* data, unsigned int rows, unsigned int cols, bool normalise)
{
  if(classes.size() != rows || !classes.size()){
    cerr << "ND_Classifier::setTrainingSet : bad rows or classes size : " << rows << " : " << classes.size() << endl;
    return(false);
  }
  delete trainData;
  trainData = data;
  dims = cols;
  t_size = rows;
  t_classes = classes;
  if(normalise)
    normaliseData(data, rows, cols);
  setClass();
  return(true);
  
}

void ND_Classifier::normaliseData(float* data, unsigned int r_no, unsigned int d_no)
{
  if(r_no < 2)
    return;
  vector<float> sum(d_no, 0);
  vector<float> sqSum(d_no, 0);
  for(uint r=0; r < r_no; ++r){
    for(uint c=0; c < d_no; ++c){
      sum[c] += data[ r * d_no + c ];
      sqSum[c] += data[ r * d_no + c ] * data[ r * d_no + c ];
    }
  }
  vector<float> SS(d_no, 0);
  vector<float> mean(d_no, 0);
  for(uint i=0; i < d_no; ++i){
    SS[i] = sqSum[i] - (sum[i] * sum[i])/(float)r_no;
    mean[i] = sum[i] / float(r_no);
    if(SS[i] <= 0){
      cerr << "ND_Classifier::normaliseData negative or 0 SS obtained. ABORTING NORMALISATION. " << endl;
      return;
    }
    // convert SS to sd_dev ..
    SS[i] = sqrt( SS[i]/float(r_no - 1) );
  }
  // then go through and convert data by the usual v - mean(v) / std.
  for(uint r=0; r < r_no; ++r){
    for(uint c=0; c < d_no; ++c)
      data[r * d_no + c] = (data[r * d_no + c] - mean[c]) / SS[c];
  }
  
}

float* ND_Classifier::classify(float* data, unsigned int rows, unsigned int cols, vector<int>& classes, bool normalise)
{
  if(!t_size){
    cerr << "ND_Classifier::classify : No training set specified" << endl;
    return(0);
  }

  if(cols != dims){
    cerr << "ND_Classifier::classify dim no of data does not match training data: " << cols << " : " << dims << endl;
    return(0);
  }
  if(!rows || !cols){
    cerr << "ND_Classifier::null data set specified : " << rows << " x " << cols << endl;
    return(0);
  }
  if(normalise)
    normaliseData(data, rows, cols);
  
  classes = class_set;
  float* proxData = new float[rows * class_set.size()];

  for(uint r_no = 0; r_no < rows; ++r_no)
    classify(data + (dims * r_no), proxData + (class_set.size() * r_no));
  
  return(proxData);
}

void ND_Classifier::classify(float* row, float* proxData){
  memset((void*)proxData, 0, sizeof(float)*class_set.size());
  float f = 0.66; // a factor that flattens the sigmoid curve. (The bigger the sharper)
  for(uint i=0; i < t_size; ++i){
    float d = 0;
    for(uint j=0; j < dims; ++j)
      d += (row[j] - trainData[i * dims + j]) * (row[j] - trainData[i * dims + j]);
    proxData[ class_offsets[t_classes[i]] ] += expf(1 - (d * d * f * f));
  }
}

void ND_Classifier::setClass(){
  set<int> cl;
  class_offsets.clear();
  class_sizes.clear();
  for(uint i=0; i < t_classes.size(); ++i){
    cl.insert(t_classes[i]);
    if(!class_sizes.count(t_classes[i]))
      class_sizes[ t_classes[i] ] = 0;
    class_sizes[ t_classes[i] ]++;
  }
  class_set.resize(cl.size());
  class_set.assign(cl.begin(), cl.end());
  for(uint i=0; i < class_set.size(); ++i)
    class_offsets[i] = class_set[i];
}
