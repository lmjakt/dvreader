#ifndef IMSTACK_H
#define IMSTACK_H

#include <vector>
#include <string>
#include "../dataStructs.h"

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
  ~ImStack();

  void addChannel(float* data, channel_info& ch_info);
  bool setData(float* data, unsigned int ch);
  std::string description();
  float* stack(unsigned int ch);
  // image() does not create a new float, but merely passes on the appropriate address
  float* image(unsigned int ch, int z); // uses global z coordinate
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

 private:
  std::vector<float*> imData;
  std::vector<channel_info> channels;
  int xo, yo, zo; // offsets or the position
  unsigned int width, height, depth;

};

#endif
