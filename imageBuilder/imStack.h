#ifndef IMSTACK_H
#define IMSTACK_H

#include <vector>
#include <string>
#include "../dataStructs.h"
#include "stack_info.h"

class ImStack
{
 public:
  ImStack(float** data, std::vector<channel_info>& ch_info, int x, int y, int z, 
	  unsigned int w, unsigned int h, unsigned int d);
  ImStack(float* data, channel_info& ch_info, int x, int y, int z, 
	  unsigned int w, unsigned int h, unsigned int d);
  ImStack(std::vector<float*>& data, std::vector<channel_info>& ch_info, int x, int y, int z, 
	  unsigned int w, unsigned int h, unsigned int d);
  ImStack(int x, int y, int z, unsigned int w, unsigned int h, unsigned int d);
  ImStack(ImStack& imstack);
  ~ImStack();

  stack_info info();
  void addChannel(float* data, channel_info ch_info);
  bool setData(float* data, unsigned int ch);
  bool exportAscii(std::string f_name);
  std::string description();
  bool subtract(ImStack* b, unsigned int ch, unsigned int b_ch, float mult, bool allowNeg, int xoff=0, int yoff=0, int zoff=0);
  float* stack(unsigned int ch);
  // image() does not create a new float, but merely passes on the appropriate address
  float* image(unsigned int ch, int z); // uses global z coordinate
  float* l_image(unsigned int ch, int z); // uses local z coordinate
  // orthogonal slice functions create new float*s which will need to be deleted by the caller.
  float* xz_slice(unsigned int ch, unsigned int ypos, int& slice_width, int& slice_height);  // uses local parameters
  float* yz_slice(unsigned int ch, unsigned int xpos, int& slice_width, int& slice_height);  // uses local parameters

  channel_info cinfo(unsigned int ch);
  bool set_sandb(unsigned int wi, float scale, float bias);
  unsigned int ch();
  void pos(int& x, int& y, int& z);
  void dims(unsigned int& w, unsigned int& h, unsigned int& d);
  int xp();
  int yp();
  int zp();
  unsigned int w();
  unsigned int h();
  unsigned int d();

  // completely unchecked access code. Inline for speed. Use carefully
  float lv(unsigned int wi, int x, int y, int z){
    return(imData[wi][z * width * height + y * width + x]);
  }
  float lv(unsigned int wi, unsigned int offset){
    return(imData[wi][offset]);
  }

 private:
  std::vector<float*> imData;
  std::vector<channel_info> channels;
  int xo, yo, zo; // offsets or the position
  unsigned int width, height, depth;

};

#endif
