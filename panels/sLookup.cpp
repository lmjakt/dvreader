#include "sLookup.h"
#include "mt_f_lookup.h"
#include "mt_rgb_lookup.h"


SLookup::SLookup(float scale, float bias, float r, float g, float b, unsigned short thread_no, unsigned int mx)
  : scale(scale), bias(bias), red(r), green(g), blue(b), thread_no(thread_no), mx_l(mx)
{
  setMx(mx_l);
  setPars(bias, scale);
  setColor(red, green, blue);
  if(!thread_no)
    thread_no = 1;
  for(unsigned int i=0; i < thread_no; ++i){
    flooks.push_back(new mt_f_lookup(max_ushort));
    rgblooks.push_back(new mt_rgb_lookup(max_ushort));
  }
}

SLookup::~SLookup()
{
  for(unsigned int i=0; i < thread_no; ++i){
    delete(flooks[i]);
    delete(rgblooks[i]);
  }
}

void SLookup::setPars(channel_info& chi)
{
  bool v_changed = false;
  bool c_changed = false;
  if((unsigned int)chi.maxLevel != mx_l){
    mx_l = (unsigned int)chi.maxLevel;
    v_changed = true;
  }
  if(chi.bias != bias || chi.scale != scale || chi.color.r != red || chi.color.g != green || chi.color.b != blue){
    bias = chi.bias;
    scale = chi.scale;
    red = chi.color.r;
    green = chi.color.g;
    blue = chi.color.b;
    c_changed = true;
  }
  if(v_changed){
    setMx(mx_l);
    return;
  }
  if(c_changed)
    setPars(scale, bias);
}

void SLookup::setMx(unsigned int mx)
{
  mx_l = mx;
  for(unsigned int i=0; i < max_ushort; ++i)
    lu_float[i] = float(i)/float(mx_l-1);
  setColor(red, green, blue);
}

void SLookup::setPars(float scl, float bs)
{
  scale = scl;
  bias = bs;
  setColor(red, green, blue);
}

void SLookup::setColor(float r, float g, float b)
{
  red = r;
  green = g;
  blue = b;
  // the simple equation I have been using is bias + scale * value
  // hmm 
  float v;
  for(unsigned int i=0; i < max_ushort; ++i){
    v = bias + scale * lu_float[i];
    v = v < 0 ? 0 : v;
    lu_red[i] = red * v;
    lu_green[i] = green * v;
    lu_blue[i] = blue * v;
  }
}

bool SLookup::toFloat(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
		      float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
		      unsigned int read_width, unsigned int read_height)
{
  if(read_width > s_w)
    return(false);
  if(read_width > d_w)
    return(false);

  unsigned int th_read_height = read_height / thread_no;
  unsigned int th_extra = read_height % thread_no;
  
  unsigned int dy = 0;
  for(unsigned int i=0; i < thread_no; ++i){
    unsigned int rh = i ? th_read_height : th_read_height + th_extra;
    flooks[i]->toFloat(source, s_x, s_y + dy, s_w,
		       dest, d_x, d_y + dy, d_w,
		       read_width, rh, lu_float);
    dy += rh;
  }
  for(unsigned int i=0; i < thread_no; ++i)
    flooks[i]->wait();
  return(true);
}


bool SLookup::addToRGB_f(unsigned short* source, unsigned int s_x, unsigned int s_y, unsigned int s_w,
			 float* dest, unsigned int d_x, unsigned int d_y, unsigned int d_w,
			 unsigned int read_width, unsigned int read_height, float* contribMap)
{
  if(read_width > s_w)
    return(false);
  if(read_width > d_w)
    return(false);

  if(!red && !blue && !green)
    return(true);

  unsigned int th_read_height = read_height / thread_no;
  unsigned int th_extra = read_height % thread_no;
  
  unsigned int dy = 0;
  for(unsigned int i=0; i < thread_no; ++i){
    unsigned int rh = i ? th_read_height : th_read_height + th_extra;
    rgblooks[i]->toRGB(source, s_x, s_y + dy, s_w,
		       dest, d_x, d_y + dy, d_w,
		       read_width, rh, 
		       lu_red, lu_green, lu_blue, contribMap);
    dy += rh;
  }
  for(unsigned int i=0; i < thread_no; ++i)
    rgblooks[i]->wait();
  return(true);
}
