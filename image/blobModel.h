#ifndef BLOBMODEL_H
#define BLOBMODEL_H

#include <vector>
#include <QMutex>

struct blob;

struct bmPoint {
  float dxy;
  float dz;
  float pp;  // peak value proportion
  bmPoint(){
    dxy = dz = pp = 0;
  }
  bmPoint(float xy, float z, float p){
    dxy = xy;
    dz = z;
    pp = p;
  }
};

// Declare a BlobModel (and an initial position, thoug this seams not to matter)
// Add blobs to the model, by setting the peak position and the peak value, followed
// by the points in the model.

class BlobModel {
 public:
  BlobModel(int x, int y, int z, float pv, int xy_radius, int z_radius);
  BlobModel(int xy_radius, int z_radius);
  ~BlobModel();
  void setPeak(int x, int y, int z, float pv);
  void setPeak(float x, float y, float z, float pv);  // for a higher-resolution model
  bool addPoint(int x, int y, int z, float v);
  float* model(int& s_range, int& z_radius);  // sets s_range * (1 + 2*z_radius) and returns a float*, containing the model 
  float* model(int& s_range, int& z_radius, unsigned int res_mult);  // makes a map with an extended resolution.  
  std::vector<bmPoint> modelPoints();
  void lockMutex();
  void unlockMutex();
 private:
  float xyRadius;
  float zRadius;
  float peakValue;
  float xp, yp, zp;  // the peak position, in float terms.
  QMutex mutex;      // use lockMutex and unlockMutex (because we use setPeak, then addPoint many times).
  std::vector<bmPoint> points;

  float dist(int x, int y);
  

};


#endif
