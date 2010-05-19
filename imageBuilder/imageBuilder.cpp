#include "imageBuilder.h"
#include "panels/fileSet.h"
#include "opengl/glImage.h"
#include "../image/two_d_background.h"
#include "../tiff/tiffReader.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>

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
  // We need two short buffers:
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


bool ImageBuilder::addBackground(unsigned int wi, unsigned int slice, color_map cm)
{

  bool ok;
  unsigned short* sb = background(wi, slice, ok);
  if(!ok){
    cerr << "ImageBuilder::addBackground didn't obtain background" << endl;
    delete sb;
    return(false);
  }
  
  // modify the current rgbData
  channel_info ci=channels[wi];
  ci.color = cm;
  toRGB(sb, rgbData, ci, data->pwidth() * data->pheight() );
  delete(sb);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::subBackground(unsigned int wi, unsigned int slice){
  bool ok;
  unsigned short* sb = background(wi, slice, ok);
  if(!ok){
    cerr << "ImageBuilder::subBackground didn't obtain background" << endl;
    delete sb;
    return(false);
  }
  channel_info ci=channels[wi];
  ci.color.r = -ci.color.r;
  ci.color.g = -ci.color.g;
  ci.color.b = -ci.color.b;
  toRGB( sb, rgbData, ci, data->pwidth() * data->pheight() );
  delete(sb);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::addMCP(unsigned int wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  if(wi >= channels.size()){
    cerr << "ImageBuilder::addMCP specified channel too large: " << wi << endl;
    return(false);
  }
  // We need two short buffers:
  unsigned short* contrast_projection = buildShortMCPProjection(sBuffer, wi, beg, end);
  toRGB(contrast_projection, rgbData, channels[wi], data->pwidth() * data->pheight());
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

bool ImageBuilder::setColor(unsigned int wi, float r, float g, float b)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setColor waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].color = color_map(r, g, b);
  return(true);
}

bool ImageBuilder::setScaleAndBias(unsigned int wi, float b, float s)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setScaleAndBias waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].bias = b;
  channels[wi].scale = s;
  return(true);
}

bool ImageBuilder::setMaxLevel(unsigned int wi, float ml)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setMaxLevel waveIndex is larger than channel size: " << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].maxLevel = ml;
  return(true);
}

bool ImageBuilder::setBackgroundPars(unsigned int wi, int cw, int ch, int cd, float qnt, bool sub)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::setBackgroundPars waveIndex is larger than channel size: "
	 << wi << " >= " << channels.size() << endl;
    return(false);
  }
  channels[wi].bgPar = backgroundPars(cw, ch, cd, qnt);
  channels[wi].bg_subtract = sub;
  return(true);
}

// makes it dark.. 
void ImageBuilder::resetRGB(){
  memset(rgbData, 0, sizeof(float) * data->pwidth() * data->pheight() * 3);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
}

void ImageBuilder::reportParameters(){
  for(uint i=0; i < channels.size(); ++i){
    cout << i << " : " << channels[i].finfo.excitation << " --> " << channels[i].finfo.emission << "\t: " << channels[i].finfo.exposure << endl;
    cout << channels[i].maxLevel << " : " << channels[i].bias << " : " << channels[i].scale << "\trgb: "
	 << channels[i].color.r << "," << channels[i].color.g << "," << channels[i].color.b << endl;
  }
}

void ImageBuilder::exportTiff(QString fname)
{
  TiffReader tr;
  if(!tr.makeFromRGBFloat(rgbData, data->pwidth(), data->pheight())){
    cerr << "ImageBuilder::exportTiff unable to make tiff from RGB" << endl;
    return;
  }
  tr.writeOut(fname.toStdString());
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
    memset((void*)sb, 0, sizeof(short) * data->pwidth() * data->pheight());
    if( data->readToShort(sb, 0, 0, i, data->pwidth(), data->pheight(), wi) ){
      got_something = true;
      if(channels[wi].bg_subtract)
	sub_slice_bg(wi, i, sb);
      maximize(p_buffer, sb, data->pwidth() * data->pheight() );
    }
  }
  return(got_something);
}

// Having a separate function seems unnecessary. But the coding might be simpler
unsigned short* ImageBuilder::buildShortMCPProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end)
{
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  unsigned short* image_buffer = new unsigned short[data->pwidth() * data->pheight()];
  unsigned short* current_contrast = new unsigned short[data->pwidth() * data->pheight()];
  unsigned short* contrast_projection = new unsigned short[data->pwidth() * data->pheight()];
  memset((void*)p_buffer, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  memset((void*)contrast_projection, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  memset((void*)current_contrast, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
  for(unsigned int i=beg; i <= end; ++i){
    memset((void*)image_buffer, 0, sizeof(unsigned short) * data->pwidth() * data->pheight());
    if( data->readToShort(image_buffer, 0, 0, i, data->pwidth(), data->pheight(), wi) ){
      if(channels[wi].bg_subtract)
	sub_slice_bg(wi, i, image_buffer);
      make_contrast(image_buffer, current_contrast, data->pwidth(), data->pheight());
      maximize_by_cc(p_buffer, image_buffer, contrast_projection, current_contrast, data->pwidth() * data->pheight() );
    }
  }
  delete image_buffer;
  delete current_contrast;
  delete contrast_projection;
  //  return(contrast_projection);
  return(p_buffer);
}


void ImageBuilder::maximize(unsigned short* a, unsigned short* b, unsigned long l)
{
  for(unsigned short* d=a; d < (a + l); ++d){
    *d = *d > *b ? *d : *b;
    ++b;
  }
}

//  cc = current contrast, cp=contrast projection, c=current image, projection=image projection. 
void ImageBuilder::maximize_by_cc(unsigned short* projection, unsigned short* ci, 
				  unsigned short* cp, unsigned short* cc, unsigned long l)
{
  for(unsigned short* d=projection; d < (projection + l); ++d){
    // cout << *d << "\t" << *ci << "\t" << *cp << "\t" << *cc << endl;
    if(*cc > *cp){
      *cp = *cc;
      *d = *ci;
    }
    ++cc;
    ++cp;
    ++ci;
  }
}

// I have some doubts as to whether this function will do the correct thing..
// but lets wait and see.
void ImageBuilder::make_contrast(unsigned short* image_data, unsigned short* contrast_data, unsigned int w, unsigned int h)
{
  memset((void*)contrast_data, 0, sizeof(unsigned short)*w*h);
  // cheat a bit. don't set any contrast data for the outermost pixels (always set to 0).
  for(uint y=1; y < (h-1); ++y){
    for(uint x=1; x < (w-1); ++x){
      unsigned short v = image_data[ y * w + x ];
      unsigned short d = 0;
      unsigned short d2 = 0;
      for(int dx=-1; dx <= +1; ++dx){
	for(int dy=-1; dy <= +1; ++dy){
	  d2 = abs(v - image_data[ (dy+y)*w + (x+dx)]);
	  d = d2 > d ? d2 : d;
	}
      }
      contrast_data[ y * w + x] = d;
    }
  }
}

void ImageBuilder::sub_slice_bg(unsigned int wi, unsigned int slice, unsigned short* sb)
{
  bool ok;
  unsigned short* bg = background(wi, slice, ok);
  if(!ok){
    cerr << "ImageBuilder::sub_slice_bg didn't obtain background" << endl;
    delete bg;
    return;
  }
  for(int i=0; i < data->pwidth() * data->pheight(); ++i)
    sb[i] = sb[i] > bg[i] ? sb[i] - bg[i] : 0;
  delete bg;
}

unsigned short* ImageBuilder::background(unsigned int wi, unsigned int slice, bool& ok)
{  

  unsigned short* sb = new unsigned short[ data->pwidth() * data->pheight()];
  memset((void*)sb, 0, sizeof(short)*data->pwidth() * data->pheight());
  if(wi >= channels.size()){
    cerr << "ImageBuilder::addBackground waveIndex too large: " << wi << endl;
    ok = false;
    return(sb);
  }
  if( !data->readToShort(sb, 0, 0, slice, data->pwidth(), data->pheight(), wi) ){
    cerr << "ImageBuilder::addBackground unable to readToShort" << endl;
    ok = false;
    return(sb);
  }
  Two_D_Background bg;
  backgroundPars& bgp = channels[wi].bgPar;
  bg.setBackground(bgp.pcntile, bgp.x_m, bgp.y_m, data->pwidth(), data->pheight(), sb);
  // reuse sb
  unsigned short* sb_ptr = sb;
  for(int y=0; y < data->pwidth(); ++y){
    for(int x=0; x < data->pheight(); ++x){
      (*sb_ptr) = (unsigned short)bg.bg(x, y);
      ++sb_ptr;
    }
  } 
  ok = true;
  return(sb);
}

void ImageBuilder::toRGB(unsigned short* sb, float* rgb, channel_info& ci, unsigned long l)
{
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
    col = 0;
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
  memset((void*)dest, 0, sizeof(float)*d_height*d_width);
  float* source_beg = source + (s_y * s_width) + s_x;
  unsigned int copy_width = (s_x + d_width <= s_width) ? d_width : (s_width - s_x);
  for(unsigned int y=0; y < d_height && (s_y + y < s_height); ++y)
    memcpy((void*)(dest + d_width * y), (const void*)(source_beg + y * s_width), sizeof(float) * copy_width);
  
}
