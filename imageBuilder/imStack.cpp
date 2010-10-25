#include "imStack.h"
#include "../dataStructs.h"
#include <sstream>

using namespace std;

ImStack::ImStack(float** data, vector<channel_info>& ch_info, int x, int y, int z,
		 unsigned int w, unsigned int h, unsigned int d)
{

  channels = ch_info;
  for(uint i=0; i < channels.size(); ++i)
    imData.push_back( data[i * w * h * d] );
  xo = x; yo = y; zo = z;
  width = w; height = h; depth = d;
}

ImStack::ImStack(float* data, channel_info& ch_info, int x, int y, int z, 
		 unsigned int w, unsigned int h, unsigned int d){
  channels.push_back(ch_info);
  imData.push_back(data);
  xo = x; yo = y; zo = z;
  width = w; height = h; depth = d;
}

ImStack::ImStack(vector<float*>& data, vector<channel_info>& ch_info, int x, int y, int z,
		 unsigned int w, unsigned int h, unsigned int d)
{
  channels = ch_info;
  imData = data;
  xo = x; yo = y; zo = z;
  width = w; height = h; depth = d;
}

ImStack::ImStack(int x, int y, int z, unsigned int w, unsigned int h, unsigned int d)
{
  xo = x; yo = y; zo = z;
  width = w; height = h; depth = d;
}

ImStack::~ImStack()
{
  for(uint i=0; i < imData.size(); ++i)
    delete []imData[i];
}

void ImStack::addChannel(float* data, channel_info& ch_info)
{
  imData.push_back(data);
  channels.push_back(ch_info);
}

bool ImStack::setData(float* data, unsigned int ch)
{
  if(ch >= imData.size())
    return(false);
  delete imData[ch];
  imData[ch] = data;
  return(true);
}

string ImStack::description(){
  ostringstream ost;
  ost << xo << "," << yo << "," << zo << "\t"
      << width << "," << height << "," << depth
      << "\t" << channels.size() << " channels";
  cout << "Description: " << ost.str() << endl;
  return(ost.str());
}

float* ImStack::stack(unsigned int ch)
{
  if(ch >= imData.size())
    return(0);
  return(imData[ch]);
}

float* ImStack::image(unsigned int ch, int z)
{
  if(ch >= channels.size())
    return(0);
  if(z < zo || z >= (zo + (int)depth))
    return(0);
  return( imData[ch] + (z - zo) * width * height );
}

float* ImStack::xz_slice(unsigned int ch, unsigned int ypos, int& slice_width, int& slice_height)
{
  if(ypos >= height || ch >= imData.size()){
    slice_width = 0; slice_height = 0;
    return(0);
  }
  slice_width = width;
  slice_height = depth;
  float* slice = new float[ width * depth ];
  float* dest = slice;
  float* source = imData[ch] + (ypos * width);
  for(uint z=0; z < depth; ++z){
    memcpy((void*)dest, (void*)source, sizeof(float) * width);
    source += (width * height);
    dest += width;
  }
  return(slice);
}

float* ImStack::yz_slice(unsigned int ch, unsigned int xpos, int& slice_width, int& slice_height)
{
  if(xpos >= width || ch >= imData.size()){
    slice_width = 0; slice_height = 0;
    return(0);
  }
  slice_width = depth;
  slice_height = height;
  float* slice = new float[ height * depth ];
  // Every pixel has to be assigned seperately..
  float* dest = slice;
  for(uint y=0; y < height; ++y){
    float* source = imData[ch] + (y * width) + xpos;
    for(uint z=0; z < depth; ++z){
      *dest = *source;
      ++dest;
      source += (width * height);
    }
  }
  return(slice);
}

channel_info ImStack::cinfo(unsigned int ch)
{
  if(ch < channels.size())
    return(channels[ch]);
  channel_info c;
  return(c);
}

bool ImStack::set_sandb(unsigned int wi, float scale, float bias)
{
  if(wi >= channels.size())
    return(false);
  channels[wi].scale = scale;
  channels[wi].bias = bias;
  return(true);
}

unsigned int ImStack::ch(){
  return(channels.size());
}

void ImStack::pos(int& x, int& y, int& z)
{
  x = xo;
  y = yo;
  z = zo;
}

void ImStack::dims(unsigned int& w, unsigned int& h, unsigned int& d)
{
  w = width;
  h = height;
  d = depth;
}

int ImStack::xp()
{
  return(xo);
}

int ImStack::yp()
{
  return(yo);
}

int ImStack::zp()
{
  return(zo);
}

unsigned int ImStack::w()
{
  return(width);
}

unsigned int ImStack::h()
{
  return(height);
}

unsigned int ImStack::d()
{
  return(depth);
}
