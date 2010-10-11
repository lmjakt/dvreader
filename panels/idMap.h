#ifndef IDMAP_H
#define IDMAP_H

// Assume 64 bit operating system (that is that long = 64 bits)
// This limits the number of panels to 63 (with ids 1 - 63)
// It also turns out not to be a particularly efficient way of handling
// the problem. It would be better to set up the contribMaps initially,
// and then setting the weights directly on the basis of the position.
// I would need to be very careful in coding that though, and I don't quite
// feel up to it at the moment. 
// sets up the id's at the given positions. 

#include <map>
#include <vector>
typedef unsigned int uint;
typedef unsigned long ulong;
class FrameStack;

class IdMap {
 public:
  IdMap(int x, int y, uint w, uint h);
  ~IdMap();
  
  bool setId(ulong id, int x, int y, int w, int h);
  bool count(ulong id, int x, int y);  // is it a member
  int count(int x, int y);  // number of members at this position.
  ulong id(int x, int y);
  void resetMap();
  void reset(int x, int y, uint w, uint h);
  void pos(int& x, int& y);
  void dims(int& w, int& h);
  float* paintCountsToRGB(int& w, int& h, float maxCount);
  void setContribMaps(std::map<ulong, FrameStack*> idMap, float pixelFactor);

 private:
  int x_pos;
  int y_pos;
  int width, height;
  ulong* map;

  std::vector<ulong> components(ulong ul);
  std::vector<FrameStack*> frameStacks(ulong ul, std::map<ulong, FrameStack*> idMap);
  float calculateWeights(int x, int y, FrameStack* frame, 
			 std::vector<FrameStack*> stacks, float pixelFactor);
};

#endif
