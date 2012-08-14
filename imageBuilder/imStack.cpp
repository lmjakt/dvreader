#include "imStack.h"
#include "../dataStructs.h"
#include <sstream>
#include <string.h>
#include <fstream>


using namespace std;

ImStack::ImStack(float** data, vector<channel_info>& ch_info, int x, int y, int z,
		 unsigned int w, unsigned int h, unsigned int d)
{

  channels = ch_info;
  for(uint i=0; i < channels.size(); ++i)
    imData.push_back( data[i] );
  //    imData.push_back( data[i * w * h * d] );
  xo = x; yo = y; zo = z;
  width = w; height = h; depth = d;
}

ImStack::ImStack(float* data, channel_info ch_info, int x, int y, int z, 
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

ImStack::ImStack(ImStack& imStack)
{
  imData.resize(imStack.imData.size());
  channels = imStack.channels;
  xo = imStack.xo;
  yo = imStack.yo;
  zo = imStack.zo;
  width = imStack.width;
  height = imStack.height;
  depth = imStack.depth;
  unsigned long l = width * height * depth;
  for(unsigned int i=0; i < imData.size(); ++i){
    imData[i] = new float[ l ];
    memcpy((void*)imData[i], (void*)imStack.imData[i], sizeof(float) * l);
  }
}

ImStack::~ImStack()
{
  for(uint i=0; i < imData.size(); ++i){
    delete []imData[i];
    imData[i] = 0;
  }
}

stack_info ImStack::info()
{
  stack_info sinfo;
  sinfo.x = xo;
  sinfo.y = yo;
  sinfo.z = zo;
  sinfo.w = width;
  sinfo.h = height;
  sinfo.d = depth;
  for(uint i=0; i < channels.size(); ++i)
    sinfo.channels.push_back(channels[i].wave_index);
  return(sinfo);
}

void ImStack::addChannel(float* data, channel_info ch_info)
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

bool ImStack::exportAscii(string f_name)
{
  ofstream out(f_name.c_str());
  if(!out)
    return(false);
  out << "z\ty\tx";
  for(unsigned int i=0; i < channels.size(); ++i){
    out << "\t" << channels[i].finfo.excitation << "-" << channels[i].finfo.emission;
  }
  out << "\n";
  for(unsigned int z=0; z < depth; ++z){
    for(unsigned int y=0; y < height; ++y){
      for(unsigned int x=0; x < width; ++x){
	out << z << "\t" << y << "\t" << x;
	for(unsigned int c=0; c < imData.size(); ++c){
	  out << "\t" << imData[c][ z * width * height + y * width + x];
	}
	out << "\n";
      }
    }
  }
  out.close();
  return(true);
}

bool ImStack::subtract(ImStack* b, unsigned int ch, unsigned int b_ch, float mult, bool allowNeg, int xoff, int yoff, int zoff)
{
  if(ch >= imData.size() || b_ch >= b->imData.size()){
    cerr << "ImStack::subtract unknown channel" << ch << endl;
    return(false);
  }
  if(width != b->width || height != b->height || depth != b->depth){
    cerr << "ImStack::subtract incompatible dimensions" << endl;
    return(false);
  }
  // xoff means that we the subtract this[0] - b[xoff]
  // since xoff can be negative we have to be careful.. 
  // we have to make sure that we dont' overstep boundaries..
  int xbeg = xoff < 0 ? -xoff : xoff;
  int ybeg = yoff < 0 ? -yoff : yoff;
  int zbeg = zoff < 0 ? -zoff : zoff;

  int xend = xoff > 0 ? (int)width - xoff : (int)width;
  int yend = yoff > 0 ? (int)height - yoff : (int)height;
  int zend = zoff > 0 ? (int)depth - zoff : (int)depth;

  for(int z=zbeg; z < zend; ++z){
    for(int y=ybeg; y < yend; ++y){
      float* minuend = imData[ch] + (z * width * height + y * width);
      float* subtrahend = b->imData[ch] + ( (z+zoff) * width * height + (y+yoff) * width + xoff);
      for(int x=xbeg; x < xend; ++x){
	(*minuend) -= mult * (*subtrahend);
	if(!allowNeg)
	  (*minuend) = (*minuend) < 0 ? 0 : (*minuend);
	++minuend;
	++subtrahend;
      }
    }
  }

  // unsigned long l = width * height * depth;
  // if(allowNeg){
  //   for(unsigned long i=0; i < l; ++i)
  //     imData[ch][i] -= (mult * b->imData[b_ch][i]);
  // }else{
  //   for(unsigned long i=0; i < l; ++i){
  //     imData[ch][i] -= (mult * b->imData[b_ch][i]);
  //     imData[ch][i] = imData[ch][i] > 0 ? imData[ch][i] : 0;
  //   }
  // }
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

// uses a global z-coordinate
float* ImStack::image(unsigned int ch, int z)
{
  if(ch >= channels.size())
    return(0);
  if(z < zo || z >= (zo + (int)depth))
    return(0);
  return( imData[ch] + (z - zo) * width * height );
}

// uses a local z-coordinate
float* ImStack::l_image(unsigned int ch, int z){
  if(ch >= channels.size())
    return(0);
  if((uint)z >= depth)
    return(0);
  return( imData[ch] + z * width * height );
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

std::vector<channel_info> ImStack::c_info()
{
  return(channels);
}

bool ImStack::setChannelInfo(channel_info ch, unsigned int wi)
{
  if(wi >= channels.size())
    return(false);
  channels[wi] = ch;
  return(true);
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
