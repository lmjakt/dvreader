#ifndef CENTERFINDER
#define CENTERFINDER

#include <vector>

struct cf_circle {
  int x, y;
  int radius;
  cf_circle(){
    x = y = radius = 0;
  }
  cf_circle(int xp, int yp, int r){
    radius = r;
    x = xp;
    y = yp;
  }
};

// Finds the center on the assumption that the center has an overall stronger signal.
// Uses a series of overlapping disks to find central areas with the strongest signal.
// Having thought about it for a bit, I suspect one might as well use squares; but
// having already written the makeDisc function, I might as well use it. Maybe it's
// more accurate.. 
class CenterFinder {
 public:
  CenterFinder(float* img, int width, int height, std::vector<int> radiuses);
  std::vector<cf_circle> findCenter();
  // This is almost a functoid. Does not delete img it was given..
 private:
  float* image;
  int imWidth, imHeight;
  std::vector<cf_circle> circles;
  char* makeDisc(int radius);
  float scanDisc(char* disc, int radius, int xo, int yo);
};

#endif
