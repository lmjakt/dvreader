#ifndef IDMAP_H
#define IDMAP_H

// Assume 64 bit operating system (that is that long = 64 bits)

// sets up the id's at the given positions. 

typedef unsigned int uint;
typedef unsigned long ulong;

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

 private:
  int x_pos;
  int y_pos;
  int width, height;

  ulong* map;

};

#endif
