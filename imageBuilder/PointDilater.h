#ifndef POINTDILATER_H
#define POINTDILATER_H

// a class that takes a set of blob_sets and uses
// these to fill an imStack object with counts of
// dilation. To see if this can be used to segment nuclei

// Unfortunately ImStack uses a float, so incrementing this has
// potentially strange effects. It might be better to start with
// an unsigned short and convert to float afterwards. Hmm.

#include <vector>
#include "blob_set.h"

class ImStack;

class PointDilater {
 public:
  PointDilater();
  ~PointDilater();

  ImStack* dilate(int x, int y, int z,
		  int w, int h, int d,
		  std::vector<blob_set> blobs,
		  int xr, int yr, int zr);
  

};

#endif
