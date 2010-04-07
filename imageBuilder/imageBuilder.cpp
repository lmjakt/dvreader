#include "imageBuilder.h"
#include "panels/fileSet.h"
#include "opengl/glImage.h"
#include <string.h>
#include <iostream>

using namespace std;

ImageBuilder::ImageBuilder(FileSet* fs, vector<channel_info>& ch)
{
  data = fs;
  texture_size = 1024;  // this must be an even 2^n
  int texColNo = (data->pwidth() % texture_size) ? 1 + (data->pwidth() / texture_size) : data->pwidth() / texture_size;
  int texRowNo = (data->pheight() % texture_size) ? 1 + (data->pheight() / texture_size) : data->pheight() / texture_size;

  image = new GLImage(texColNo, texRowNo, texture_size);
  image->resize(texture_size, texture_size);
  
  rgbData = new float[ data->pwidth() * data->pheight() * 3];
  sBuffer = new unsigned short[ data->pwidth() * data->pheight() ];
  channels = ch;
  for(uint i=0; i < channels.size(); ++i)
    channels[i].finfo = data->channelInfo(i);
}

ImageBuilder::~ImageBuilder()
{
  delete image;
  delete rgbData;
  delete sBuffer;
}

bool ImageBuilder::buildSlice(std::set<unsigned int> wi, unsigned int slice)
{
  memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  for(uint i=0; i < channels.size(); ++i)
    channels[i].include = (bool)wi.count(i);
  if( !data->readToRGB(rgbData, 0, 0, data->pwidth(), data->pheight(), slice, channels) )
    return(false);

  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::buildProjection(std::vector<unsigned int> wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  // We need to short buffers:
  bool ok = true;
  memset((void*)rgbData, 0, sizeof(float) * data->plength() * 3);
  for(unsigned int i=0; i < wi.size(); ++i){
    if(wi[i] >= channels.size()){
      cerr << "ImageBuilder::buildProjection waveIndex too large : " << wi[i] << endl;
      ok = false;
      continue;
    }
    memset((void*)sBuffer, 0, sizeof(short) * data->pwidth() * data->pheight() );
    if(buildShortProjection(sBuffer, wi[i], beg, end))
      toRGB(sBuffer, rgbData, channels[wi[i]], data->pwidth() * data->pheight());
  }
  if(ok)
    setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(ok);   // not very meaningful
}

bool ImageBuilder::setColor(int wi, float r, float g, float b)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setColor waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].color = color_map(r, g, b);
  return(true);
}

bool ImageBuilder::setScaleAndBias(int wi, float b, float s)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setScaleAndBias waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].bias = b;
  channels[wi].scale = s;
  return(true);
}

bool ImageBuilder::setMaxLevel(int wi, float ml)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setMaxLevel waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].maxLevel = ml;
  return(true);
}

void ImageBuilder::reportParameters(){
  for(uint i=0; i < channels.size(); ++i){
    cout << i << " : " << channels[i].finfo.excitation << " --> " << channels[i].finfo.emission << "\t: " << channels[i].finfo.exposure << endl;
    cout << channels[i].maxLevel << " : " << channels[i].bias << " : " << channels[i].scale << "\trgb: "
	 << channels[i].color.r << "," << channels[i].color.g << "," << channels[i].color.b << endl;
  }
}

bool ImageBuilder::buildShortProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  unsigned short* sb = new unsigned short[data->pwidth() * data->pheight()];
  memset((void*)p_buffer, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  bool got_something = false;
  for(unsigned int i=beg; i <= end; ++i){
    if( data->readToShort(sb, 0, 0, i, data->pwidth(), data->pheight(), wi) ){
      got_something = true;
      maximize(p_buffer, sb, data->pwidth() * data->pheight() );
    }
  }
  return(got_something);
}

void ImageBuilder::maximize(unsigned short* a, unsigned short* b, unsigned long l){
  for(unsigned short* d=a; d < (a + l); ++d){
    *d = *d > *b ? *d : *b;
    ++b;
  }
}

void ImageBuilder::toRGB(unsigned short* sb, float* rgb, channel_info& ci, unsigned long l){
  float v;
  for(unsigned short* s=sb; s < (sb + l); ++s){
    v = ci.bias + ci.scale * ((float)(*s) / ci.maxLevel);
    if(v > 0){
      *rgb += (v * ci.color.r);
      *(rgb + 1) += (v * ci.color.g);
      *(rgb + 2) += (v * ci.color.b);
      // don't care about going too high. open GL takes care. I believe..
    }
    rgb += 3;
  }
}

void ImageBuilder::setRGBImage(float* img, unsigned int width, unsigned int height)
{
  int row = 0;
  int col = 0;
  float* subImg = new float[texture_size * texture_size * 3];
  if(!image->isVisible())
    image->show();
  while(row * texture_size < height){
    while(col * texture_size < width){
      subImage(img, subImg, 3*width, height, 
	       3*col * texture_size, row * texture_size,
	       3*texture_size, texture_size);
      //// We may need to do something a bit more clever than use texture_size
      //// below. If we don't have an exact fit we'll get some quite funny repeats.
      image->setImage(subImg, texture_size, texture_size, col, row);
      ++col;
    }
    ++row;
  }
  delete subImg;
  image->updateGL();
}

void ImageBuilder::subImage(float* source, float* dest,
			    unsigned int s_width, unsigned int s_height,
			    unsigned int s_x, unsigned int s_y,
			    unsigned int d_width, unsigned int d_height
			    )
{
  float* source_beg = source + (s_y * s_width) + s_x;
  unsigned int copy_width = (s_x + d_width <= s_width) ? d_width : (s_width - s_x);
  for(unsigned int y=0; y < d_height && (s_y + y < s_height); ++y)
    memcpy((void*)(dest + d_width * y), (const void*)(source_beg + y * s_width), sizeof(float) * copy_width);
  
}
