#ifndef BLOBMODEL_H
#define BLOBMODEL_H

#include <vector>

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


class BlobModel {
 public:
  BlobModel(int x, int y, int z, float pv, int xy_radius, int z_radius);
  void setPeak(int x, int y, int z, float pv);
  bool addPoint(int x, int y, int z, float v);
  float* model(int& s_range, int& z_radius);  // sets s_range * (1 + 2*z_radius) and returns a float*, containing the model 
 private:
  float xyRadius;
  float zRadius;
  float peakValue;
  float xp, yp, zp;  // the peak position, in float terms.
  std::vector<bmPoint> points;

  float dist(int x, int y);

};


#endif
