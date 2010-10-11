#include "imageBuilder.h"
#include "panels/fileSet.h"
#include "opengl/glImage.h"
#include "centerFinder.h"
#include "../dataStructs.h"
#include "../image/two_d_background.h"
#include "../image/spectralResponse.h"
#include "../tiff/tiffReader.h"
#include "../distchooser/distChooser.h"
#include "../distchooser/tabWidget.h"
#include "../stat/stat.h"
#include "../image/gaussian.h"
#include "../panels/stack_stats.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>

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

  distTabs = new TabWidget();
  distTabs->setCaption("ImageBuilder Distributions");
  for(uint i=0; i < channels.size(); ++i){
    channels[i].finfo = data->channelInfo(i);
    ostringstream ss;
    ss << data->channel(i);
    distributions.push_back(new DistChooser(ss.str(), 100));
    distTabs->addTab(distributions.back(), ss.str().c_str());
  }
  
  // set up map of various pipe slice functions.
  pipe_slice_functions["slice"] = &ImageBuilder::p_slice;
  pipe_slice_functions["g_blur"] = &ImageBuilder::p_gaussian;
  pipe_slice_functions["lg_blur"] = &ImageBuilder::pl_gaussian;
  pipe_slice_functions["bg_sub"] = &ImageBuilder::p_sub_bg;
  pipe_slice_functions["ch_sub"] = &ImageBuilder::p_sub_channel;
  pipe_slice_functions["p_mean"] = &ImageBuilder::p_mean_panel;

  // a set of general functions taking instances of the f_parameter class
  // as an argument.
  general_functions["set_panel_bias"] = &ImageBuilder::setPanelBias;
  general_functions["set_frame_bgpar"] = &ImageBuilder::setFrameBackgroundPars;
  general_functions["find_center"] = &ImageBuilder::findCenter;
  general_functions["f_project"] = &ImageBuilder::build_fprojection;
  general_functions["add_slice"] = &ImageBuilder::addSlice;
  //general_functions["p_mean"] = &ImageBuilder::p_mean_panel;  // may be useful to do things like blurring.

}

ImageBuilder::~ImageBuilder()
{
  delete image;
  delete []rgbData;
  delete []sBuffer;
  // I may need to delete the distributions separately, not sure,,
  delete distTabs;
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

bool ImageBuilder::gaussianSlice(set<unsigned int> wi, unsigned int slice, unsigned int g_radius)
{
  vector<unsigned int> ch;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size())
      ch.push_back(*it);
  }
  if(!ch.size()){
    cerr << "ImageBuilder::gaussianSlice no good channels selected" << endl;
    return(false);
  }
  float** images = getFloatImages(ch, slice);
  float** bl_images = new float*[ch.size()];
  for(uint i=0; i < ch.size(); ++i){
    bl_images[i] = gaussian_blur_2d(images[i], data->pwidth(), data->pheight(), g_radius);
  }
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());
  for(uint i=0; i < ch.size(); ++i)
    toRGB(bl_images[i], rgbData, channels[ch[i]], data->pwidth() * data->pheight());
  deleteFloats(images, ch.size());
  deleteFloats(bl_images, ch.size());
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  return(true);
}

// At the moment, this uses a blurred image to estimate the background.
// But it still is not behaving as well as I was hoping.
bool ImageBuilder::buildSpectrallySubtractedImage(std::set<unsigned int> wi, unsigned int slice)
{
  if(bg_spectrum.size() < 2){
    cerr << "Unable to build spectrally subtracted image. bg_spectrum too small: " << bg_spectrum.size() << endl;
    return(false);
  }
  for(map<unsigned int, float>::iterator it=bg_spectrum.begin();
      it != bg_spectrum.end(); it++){
    if(!wi.count(it->first)){
      cerr << "Unable to build spectrally subtracted image. Not enough channels specified : " << it->first << endl;
      return(false);
    }
  }
  // and select the known things.
  vector<unsigned int> ch;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size())
      ch.push_back(*it);
  }
  
  // get all the images..
  float** images = new float*[ch.size()];
  // TEMPORARY HACK TRY TO BLUR IMAGES BEFORE CALCULATING THE BACKGROUND
  float** bl_images = new float*[ch.size()];
  // keep the background spectrum in the bgr array (background responses)
  float* bgr = new float[ch.size()];
  for(uint i=0; i < ch.size(); ++i){
    images[i] = new float[ data->pwidth() * data->pheight() ];
    memset((void*)images[i], 0, sizeof(float) * data->pwidth() * data->pheight());
    data->readToFloat(images[i], 0, 0, slice, data->pwidth(), data->pheight(), 1, ch[i]);
    bl_images[i] = gaussian_blur_2d(images[i], data->pwidth(), data->pheight(), 3);
    if(bg_spectrum.count(ch[i])){
      bgr[i] = bg_spectrum[ch[i]];
    }else{
      bgr[i] = -1;
    }
  }
  // and then go through all of the pixel positions, estimate a max background value, and
  // subtract from the channel values.
  unsigned int l = data->pwidth() * data->pheight();
  float bg = 0;
  float tbg = 0;
  // The speed of the below can be improved by making sure that we only
  // go through the channels for which the background rate has been estimated.
  // To do that however, we would need another layer of index translation, which
  // makes it easier to introduce stupid bugs. So I'll leave this slow at the moment.
  for(unsigned int i=0; i < l; ++i){
    bg = -1;
    for(unsigned int j=0; j < ch.size(); ++j){
      if(bgr[j] <= 0)
	continue;
      if(bg < 0){         // has not been estimated for this pixel yet
	bg = bl_images[j][i] / bgr[j];
	continue;
      }
      tbg = bl_images[j][i] / bgr[j];
      bg = tbg < bg ? tbg : bg;
    }
    // And then subtract. This will leave one channel with a 0 value.
    // That's a little bit too harsh, and it means that we need to have
    // one empty background channel; can't have full labelling (3 colours).
    // which is a bit of a pity.
    if(bg <= 0)
      continue;
    for(unsigned int j=0; j < ch.size(); ++j){
      if(bgr[j] < 0)
	continue;
      //bl_images[j][i] /= (bg * bgr[j]);
      images[j][i] -= (bg * bgr[j]);
      //images[j][i] *= bl_images[j][i];
    }
  }
  // And then we simply have to convert this to an image of sorts..
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());
  for(uint i=0; i < ch.size(); ++i){
    toRGB(images[i], rgbData, channels[ch[i]], data->pwidth() * data->pheight());
    delete []images[i];
  }
  delete []images;
  delete []bgr;
  deleteFloats(bl_images, ch.size());
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

void ImageBuilder::build_fprojection(f_parameter& par)
{
  int x = 0;
  int y = 0;
  int width = data->pwidth();
  int height = data->pheight();
  int beg = 0;
  int end = 0;
  vector<int> waves;
  bool use_cmap=true;
  // override the defaults if specified.
  if(!par.param("beg", beg) || !par.param("end", end)){
    cerr << "build_fprojection you must specify beg (int) end (int)" << endl;
    return;
  }
  if(!par.param("wi", ',', waves)){
    cerr << "build_fprojection you must specify the wave indexes wi=1,2,3 or something" << endl;
    return;
  }
  par.param("w", width); par.param("h", height); par.param("x", x); par.param("y", y);
  par.param("cmap", use_cmap);

  bool clear = true;
  par.param("clear", clear);
  if(clear)
    memset((void*)rgbData, 0, sizeof(float) * data->plength() * 3);
  if(width <= 0 || height <= 0)
    return;
  float* pbuffer = new float[width * height];
  for(unsigned int i=0; i < waves.size(); ++i){
    if((unsigned int)waves[i] >= channels.size())
      continue;
    memset((void*)pbuffer, 0, sizeof(float) * width * height);
    if(buildFloatProjection(pbuffer, waves[i], x, y, width, height, (unsigned int)beg, (unsigned int)end, use_cmap)){
      pbuffer = modifyImage(x, y, width, height, pbuffer, par);
      toRGB(pbuffer, rgbData, channels[waves[i]], width * height, x, y, width, height);
    }
  }
  setRGBImage(rgbData, data->pwidth(), data->pheight());  // hmm 
  delete []pbuffer;
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

// Estimates the backround spectral response, and sets the values in bg_spectrum.
void ImageBuilder::paintSpectralResponse(set<unsigned int> wi, unsigned int slice)
{
  // let's make sure all channels exist
  vector<unsigned int> ch;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size())
      ch.push_back(*it);
  }
  if(!ch.size()){
    cerr << "ImageBuilder paintSpectralResponse, empty channel set" << endl;
    return;
  }
  float** images = getFloatImages(ch, slice);
  // float** images = new float*[ch.size()];
  // for(uint i=0; i < ch.size(); ++i){
  //   images[i] = new float[ data->pwidth() * data->pheight() ];
  //   memset((void*)images[i], 0, sizeof(float) * data->pwidth() * data->pheight());
  //   data->readToFloat(images[i], 0, 0, slice, data->pwidth(), data->pheight(), 1, ch[i]);
  // }
  SpectralResponse spr;
  float** bgr = spr.bg_response(images, ch.size(), data->pwidth(), data->pheight());
  if(!bgr){
    for(uint i=0; i < ch.size(); ++i)
      delete []images[i];
    delete []images;
    return;
  }

  // Then we need to paint the bgr in some way. But the question is what parameters
  // to use. The maximum background response will be close to 1, and that is likely
  // to be true for the green colors, With much lower values elsewhere.
  // we could autoscale the response, by remembering the max values.
  // Still, I would suggest using the channel thingies.
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());

  bg_spectrum.clear();
  for(uint i=0; i < ch.size(); ++i){
    toRGB(bgr[i], rgbData, channels[ch[i]], data->pwidth() * data->pheight());
    distributions[ch[i]]->setData(bgr[i], data->pwidth() * data->pheight(), 0, 1.0, true);
    bg_spectrum[ch[i]] = mode_average(bgr[i], data->pwidth() * data->pheight(), 500, 0, 1.0);
    cout << "Background response for channel " << ch[i] << " : " << bg_spectrum[ch[i]] << endl; 
    delete []bgr[i];
    delete []images[i];
  }
  delete []bgr;
  delete []images;
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  distTabs->show();
}

// ps_ch is a pseduo channel used to specify the colour and the other parameters for turning the
// background into an rgb image
void ImageBuilder::paintSpectralBackground(set<unsigned int> wi, unsigned int slice, unsigned int ps_ch)
{
  if(bg_spectrum.size() < 2){
    cerr << "Unable to build spectrally background image. bg_spectrum too small: " << bg_spectrum.size() << endl;
    return;
  }
  for(map<unsigned int, float>::iterator it=bg_spectrum.begin();
      it != bg_spectrum.end(); it++){
    if(!wi.count(it->first)){
      cerr << "Unable to build spectrally subtracted image. Not enough channels specified : " << it->first << endl;
      return;
    }
  }
  // and select the known things.
  vector<unsigned int> ch;
  vector<float> bgr;
  for(set<unsigned int>::iterator it=wi.begin(); it != wi.end(); ++it){
    if((*it) < channels.size() && bg_spectrum.count(*it)){
      ch.push_back(*it);
      bgr.push_back( bg_spectrum[*it] );
    }
  }
  if(ch.size() < 1){
    cerr << "ImageBuilder::paintSpectralBackground no appropriate channel specified" << endl;
    return;
  }
  if(ps_ch >= channels.size())
    ps_ch = 0;
  
  float** images = getFloatImages(ch, slice);
  float* sp_bg = spectralBackgroundEstimate(images, bgr, bgr.size(), data->pwidth() * data->pheight());
  memset((void*)rgbData, 0, sizeof(float) * 3 * data->pwidth() * data->pheight());
  toRGB(sp_bg, rgbData, channels[ps_ch], data->pwidth() * data->pheight());
  deleteFloats(images, ch.size());
  delete []sp_bg;
  setRGBImage(rgbData, data->pwidth(), data->pheight());
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

// buildFloatProjection uses fileSet->toFloat. This means that the biases and background projection
// parameters set for frame will be used. This means that background correction may work better around
// bordering things. It also means that contribMap will be used and we should get a smother edge blending
bool ImageBuilder::buildFloatProjection(float* p_buffer, unsigned int wi, int x, int y, int width, int height,
					unsigned int beg, unsigned int end, bool use_cmap)
{
  cout << "buildFloatProjection : " << wi << ": " << x << "," << y << "  dims: " << width << "," << height << "  " << beg << " --> " << end << endl;
  if(width <= 0 || height <= 0 || x + width > data->pwidth() || y + height > data->pheight())
    return(false);
  if(beg > end){
    unsigned int b = beg;
    beg = end;
    end = b;
  }
  float* buffer = new float[width * height];
  memset((void*)p_buffer, 0, sizeof(float) * width * height);
  bool got_something = false;
  for(unsigned int i=beg; i <= end; ++i){
    memset((void*)buffer, 0, sizeof(float) * width * height);
    if( data->readToFloat(buffer, x, y, i, width, height, 1, wi, use_cmap) ){
      got_something = true;
      maximize(p_buffer, buffer, width * height);
    }
  }
  delete []buffer;
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

void ImageBuilder::maximize(float* a, float* b, unsigned long l)
{
  for(float* d=a; d < (a + l); ++d){
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
  // slice could be used if we have a cache of some sorts.
  //bool ok;
  unsigned short* bg = background(sb, data->pwidth(), data->pheight(), channels[wi].bgPar);
  //  unsigned short* bg = background(wi, slice, ok);
  // if(!ok){
  //   cerr << "ImageBuilder::sub_slice_bg didn't obtain background" << endl;
  //   delete bg;
  //   return;
  // }
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
  for(int y=0; y < data->pheight(); ++y){
    for(int x=0; x < data->pwidth(); ++x){
      (*sb_ptr) = (unsigned short)bg.bg(x, y);
      ++sb_ptr;
    }
  } 
  ok = true;
  return(sb);
}

unsigned short* ImageBuilder::background(unsigned short* sb, unsigned int w, unsigned int h, backgroundPars& bgp)
{
  if(!w || !h){
    cerr << "ImageBuilder::background, null background specified" << endl;
    return(0);
  }
  
  Two_D_Background bg;
  bg.setBackground(bgp.pcntile, bgp.x_m, bgp.y_m, w, h, sb);
  unsigned short* bground = new unsigned short[ w * h ];
  unsigned short* bgptr = bground;
  for(unsigned int y=0; y < h; ++y){
    for(unsigned int x=0; x < w; ++x){
      (*bgptr) = bg.bg(x, y);
      ++bgptr;
    }
  }
  return(bground);
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

void ImageBuilder::toRGB(float* fb, float* rgb, channel_info& ci, unsigned long l)
{
  float v;
  for(float* f=fb; f < (fb + l); ++f){
    v = ci.bias + ci.scale * (*f);
    if(v > 0){
      *rgb += (v * ci.color.r);
      *(rgb + 1) += (v * ci.color.g);
      *(rgb + 2) += (v * ci.color.b);
    }
    rgb += 3;
  }
}

// assumes that rgb has the same dimesions as rgbData. (i.e. data->pwidth() * data->pheight())
void ImageBuilder::toRGB(float* fb, float* rgb, channel_info& ci, unsigned long l,
			 int x_off, int y_off, int width, int height, bool clear)
{
  // In any case if clear is true, then lets clear..
  if(clear)
    memset((void*)rgbData, 0, sizeof(float) * data->pwidth() * data->pheight());
  // the rgbData defaults to the size of data->pwidth() * data->pheight() 
  // make sure that the x_off + y_off etc don't cause problems.
  // for argument's sake we do allow negative offsets, but these are considered off
  // the image and will cause a shift. See below for how to handle.
  if(x_off < 0){
    width += x_off;
    x_off = 0;
  }
  if(y_off < 0){
    height += y_off;
    y_off = 0;
  }
  if(width <= 0 || height <= 0){
    cerr << "ImageBuilder::toRGB negative width or height specified : " << width << " x " << height << endl;
    return;
  }
  if(x_off >= data->pwidth() || y_off >= data->pheight()){
    cerr << "ImageBilder::toRGB coordinates outside of image: " << x_off << "," << y_off << endl;
    return;
  }
  // make sure width + x_off not bigger than data->pwidth() and so on..
  width = (x_off + width) < data->pwidth() ? width : (data->pwidth() - x_off);
  height = (y_off + height) < data->pheight() ? height : (data->pheight() - y_off);
  
  // and then line by line.. 
  float v;
  for(int dy=0; dy <  height; ++dy){
    float* src = (fb + dy * width);
    float* dst = rgb + 3 * (((y_off + dy) * data->pwidth()) + x_off);
    for(int dx = 0; dx < width; ++dx){
      v = ci.bias + ci.scale * (*src);
      if(v > 0){
	*dst += (v * ci.color.r);
	*(dst + 1) += (v * ci.color.g);
	*(dst + 2) += (v * ci.color.b);
      }
      ++src;
      dst += 3;
    }
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
  delete []subImg;
  image->updateGL();
}

void ImageBuilder::setBigImage(float* img, int source_x, int source_y, int width, int height){
  if(!image->isVisible())
    image->show();
  image->setBigImage(img, source_x, source_y, width, height);
}

void ImageBuilder::pipe_slice(std::vector<p_parameter> pars, unsigned int wi, bool reset_rgb){
  if(wi < channels.size())
    pipe_slice(pars, channels[wi], reset_rgb);
}

void ImageBuilder::pipe_slice(vector<p_parameter> pars, channel_info& ch_info, bool reset_rgb)
{
  if(!pars.size())
    return;
  if(reset_rgb)
    resetRGB();
  ps_function func = 0;
  float* img = 0;
  for(uint i=0; i < pars.size(); ++i){
    if(!pipe_slice_functions.count(pars[i].function)){
      cerr << "ImageBuilder::pipe_slice no function defined for : " << pars[i].function << endl;
      break;
    }
    func = pipe_slice_functions[pars[i].function];
    // transfer the image from the last one if we have it
    if(i){
      pars[i].setImage( pars[i-1].image() );
      pars[i].setArea( pars[i-1] );   // Hack to make sure we don't screw up the image that we're passing around
    }
    if(!(*this.*func)(pars[i])){
      cerr << "imageBuilder::pipe_slice func returned false breaking off at i: " << i << endl;
      break;
    }
    img = pars[i].image();
  }
  if(!img){
    cerr << "ImageBuilder::pipe_slice failed to get an image, doing nothing" << endl;
    return;
  }
  //////// note that toRGB will need to be rewritten to handle sub images;
  //////// Since the pipe functions will happily extract any subset of the
  //////// the overall image. 
  cout << "calling toRGB with : " << (long)img << " dims : " << pars[0].width << "," << pars[0].height << "  pos: " 
       << pars[0].x << "," << pars[0].y << endl;
  toRGB(img, rgbData, ch_info, pars[0].width * pars[0].height, pars[0].x, pars[0].y, pars[0].width, pars[0].height);
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  delete []img;
}

void ImageBuilder::general_command(f_parameter& par){
  if(!general_functions.count(par.function())){
    cerr << "ImageBuilder::general_command, unknown command : " << par.function().toAscii().constData() << endl;
    return;
  }
  cout << "ImageBuilder::general command function name is : a QString " << par.function().toAscii().constData() << endl;
  g_function func = general_functions[par.function()];
  (*this.*func)(par);
}

void ImageBuilder::getStackStats(unsigned int wi, int xb, int yb, int zb, int s_width, int s_height, int s_depth)
{
  if(wi >= channels.size()){
    cerr << "ImageBuilder::getStackStats wi is larger than channels.size()" << endl;
    return;
  }
  int col_no, row_no, pwidth, pheight;
  col_no = row_no = pwidth = pheight = 0;
  data->stackDimensions(col_no, row_no, pwidth, pheight);
  
  if(xb + s_width > pwidth || yb + s_height > pheight){
    cerr << "ImageBuilder::getStackStats illegal parameters" << endl;
    return;
  }
  vector<stack_stats> all_stats;
  for(int col=0; col < col_no; ++col){
    for(int row=0; row < row_no; ++row){
      all_stats.push_back(data->stackStats(col, row, xb, yb, zb, s_width, s_height, s_depth, wi));
      //      stack_stats stats = data->stackStats(col, row, xb, yb, zb, s_width, s_height, s_depth, wi);
      // and let's just print out some stats..
      //      cout << row << "," << col << " : "  << wi 
      //   << "\t" << stats.minimum << " -> " << stats.maximum << "  mean: " << stats.mean << "  median : " << stats.median 
      //   << "  mode : " << stats.mode << endl;
      //for(unsigned int i=0; i < stats.qntiles.size(); ++i)
      //cout << "\t" << stats.qntiles[i];
    }
  }
  unsigned int i=0;
  for(int col=0; col < col_no; ++col){
    for(int row=0; row < row_no; ++row){
      cout << col << "," << row << "\t" << all_stats[i].minimum << "-" << all_stats[i].maximum << "\t" 
	   << all_stats[i].mean << "\t" << all_stats[i].median << "\t" << all_stats[i].mode;
      for(uint j=0; j < all_stats[i].qntiles.size(); ++j)
	cout << "\t" << all_stats[i].qntiles[j];
      cout << endl;
      ++i;
    }
  }
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

// images should contain only those images for which we have a background
// response estimate contained in bgr (which must be the same size as images)
float* ImageBuilder::spectralBackgroundEstimate(float** images, vector<float> bgr, unsigned int im_no, unsigned int im_l)
{
  float* bg = new float[im_l];
  
  for(uint i=0; i < im_l; ++i){
    bg[i] = images[0][i] / bgr[0];
    for(uint j=1; j < im_no; ++j)
      bg[i] = (images[j][i] / bgr[j]) < bg[i] ? (images[j][i] / bgr[j]) : bg[i];
  }
  return(bg);
}

float** ImageBuilder::getFloatImages(vector<unsigned int> ch, unsigned int slice, bool use_cmap)
{
  return(getFloatImages(ch, slice, 0, 0, data->pwidth(), data->pheight(), use_cmap));
}

float** ImageBuilder::getFloatImages(vector<unsigned int> ch, unsigned int slice, int x, int y, int w, int h, bool use_cmap)
{
  if(!ch.size()){
    cerr << "ImageBuilder::getFloatImages no channels specified. " << endl;
    return(0);
  }
  // make sure w and h are positive and non-negative 
  if(w <= 0 || h <= 0)
    return(0);

  float** images = new float*[ch.size()];
  for(uint i=0; i < ch.size(); ++i){
    images[i] = new float[ w * h ];
    memset((void*)images[i], 0, sizeof(float) * w * h);
    data->readToFloat(images[i], x, y, slice, w, h, 1, ch[i], use_cmap);
  }
  return(images);
}

void ImageBuilder::deleteFloats(float** fl, unsigned int l){
  for(uint i=0; i < l; ++i)
    delete []fl[i];
  delete fl;
}

void ImageBuilder::toMean(float* mean, unsigned short* add, int l, float fraction){
  for(int i=0; i < l; ++i)
    mean[i] += (fraction * ((float)add[i]));
}

// sets circumference to 1.0, and adds 0.25 to internal part of the circle.
void ImageBuilder::drawCircle(float* image, int imWidth, int imHeight, int x, int y, int r){
  if(imWidth < 0 || imWidth < 0 || r < 0 || x < 0 || y < 0)
    return;
  int yb = y >= r ? r : y;
  int ye = (y + r) <= imHeight ? r : (imHeight - y) - 1;
  int xb = x >= r ? r : x;
  int xe = (x + r) <= imWidth ? r : (imWidth - x) - 1;
  for(int dy=-yb; dy <= ye; ++dy){
    for(int dx=-xb; dx <= xe; ++dx){
      float d = sqrt( (dy * dy) + (dx * dx) );
      if(d > (float)r)
	continue;
      if(((float)r - d) <= 1.0){
	image[ (y + dy) * imWidth + (x + dx) ] = 1.0;
	continue;
      }
      image[ (y + dy) * imWidth + (x + dx) ] += 0.25;
    }
  }
} 


// A simple function. Returns a float* representation of a single slice
// for a single channel. Image width and height are taken from the appropriate
// fields.
bool ImageBuilder::p_slice(p_parameter& par)
{
  if(!par.width || !par.height)
    return(false);
  int z = 0;
  int ch = 0;
  if(!par.param("z", z) || !par.param("ch", ch))
    return(false);
  float* img = par.image();
  if(!img){
    img = new float[par.width * par.height];
    par.setImage(img);
  }
  memset((void*)img, 0, sizeof(float) * par.width * par.height );
  if(!data->readToFloat(img, par.x, par.y, z, par.width, par.height, 1, (unsigned int)ch)){
    return(false);
  }
  return(true);
}

bool ImageBuilder::p_gaussian(p_parameter& par)
{
  if(!par.image()){
    cerr << "ImageBuilder::p_gaussian no image specified. Returning" << endl;
    return(false);
  }
  // extract the relevant parameters.
  float* img = par.image();
  int radius = 0;
  if(!par.param("radius", radius)){
    cerr << "ImageBuilder::p_gaussian, unable to obtain radius" << endl;
    return(false);
  }
  float* bl_image = gaussian_blur_2d(img, par.width, par.height, (unsigned int)radius);
  par.setImage(bl_image);
  return(true);
}

bool ImageBuilder::pl_gaussian(p_parameter& par)
{
  if(!par.image()){
    cerr << "ImageBuilder::pl_gaussian no image specified" << endl;
    return(false);
  }
  float* img = par.image();
  int radius = 0;
  if(!par.param("radius", radius)){
    cerr << "ImageBuilder::pl_gaussian, unable to obtain radius" << endl;
    return(false);
  }
  float* bl_image = gaussian_blur_1d(img, par.width, par.height, (unsigned int)radius);
  if(bl_image){
    par.setImage(bl_image);
    return(true);
  }
  return(false);
}

bool ImageBuilder::p_sub_bg(p_parameter& par){
  if(!par.image()){
    cerr << "ImageBuilder::p_sub_bg no image specified retunring false" << endl;
    return(false);
  }
  int xr, yr;
  float pcntile = 0;
  if(!par.param("bg_xr", xr) || !par.param("bg_yr", yr) || !par.param("bg_pcnt", pcntile)){
    cerr << "ImageBuilder::p_sub_bg Unable to get background paramters" << endl;
    return(false);
  }
  Two_D_Background bg;
  bg.setBackground(pcntile, (unsigned int)xr, (unsigned int)yr, 
		   (unsigned int)par.width, (unsigned int)par.height, par.image());
  // then go through and subtract..
  float* ptr = par.image();
  float b;
  for(int y=0; y < par.height; ++y){
    for(int x=0; x < par.height; ++x){
      b = bg.bg(x, y);
      (*ptr) = b < (*ptr) ? (*ptr) - b : 0;
      ++ptr;
    }
  }
  return(true);
}

bool ImageBuilder::p_sub_channel(p_parameter& par){
  if(!par.image()){
    cerr << "ImageBuilder::p_sub_channel Image needs to be specified in the image" << endl;
  }
  // This function makes the assumption that the img data specified will have the same
  // dimensions. This is sort of enforced by the structure of the calling functions, but
  // as it can't be checked it's a bit of a bummer.
  // Parameters required are:
  // channel 
  // multiplier
  // allow_negative values ?
  // optional parameters:
  // x and y offsets.
  if(!par.width || !par.height)
    return(false);
  int ch;
  int al_neg;
  float multiplier;
  int z;
  if(!par.param("ch", ch) || !par.param("al_neg", al_neg) || !par.param("mult", multiplier) || !par.param("z", z)){
    cerr << "ImageBuilder::p_sub_channel insufficent parameters specified" << endl;
    return(false);
  }
  int xo, yo;
  if(!par.param("xo", xo))
    xo = 0;
  if(!par.param("yo", yo))
    yo = 0;
  float* sub_image = new float[par.width * par.height];
  memset((void*)sub_image, 0, sizeof(float) * par.width * par.height);
  if(!data->readToFloat(sub_image, par.x + xo, par.y + yo, z, par.width, par.height, 1, (unsigned int)ch)){
    cerr << "ImageBuilder::p_sub_channel unable to read the data for the subtraction" << endl;
    delete []sub_image;
    return(false);
  }
  // A gaussian blur can be specified by the g_blur parameter, with radius given by the
  // second thdnngy.
  int g_radius = 0;
  float* bl_sub = 0;
  if(par.param("g_blur", g_radius))
    bl_sub = gaussian_blur_2d(sub_image, par.width, par.height, (unsigned int)g_radius);
  if(bl_sub){
    delete []sub_image;
    sub_image = bl_sub;
  }
  // then we can simply subtract sub_image from img..
  float* img = par.image();
  float* end_image = img + (par.width * par.height);
  if(al_neg){
    while(img < end_image){
      (*img) -= (*sub_image) * multiplier;
      ++img;
      ++sub_image;
    }
  }else{
    while(img < end_image){
      (*img) = (*img) > ((*sub_image) * multiplier) ? (*img) - (*sub_image) * multiplier : 0.0;
      ++img;
      ++sub_image;
    }
  }
  return(true);
}

bool ImageBuilder::p_mean_panel(p_parameter& par)
{
  int ch;
  if(!par.param("ch", ch))
    return(false);
  
  int col_begin = 0;
  int row_begin = 0;
  par.param("cb", col_begin);
  par.param("rb", row_begin);

  if((unsigned int)ch >= channels.size()){
    cerr << "ImageBuilder::p_mean_panel channel number too large" << endl;
    return(false);
  }
  // first get the appropriate dimensions..
  int col_no, row_no, pwidth, pheight;
  col_no = row_no = pwidth = pheight = 0;
  int depth = data->sectionNo();
  data->stackDimensions(col_no, row_no, pwidth, pheight);
  if(!col_no || !row_no || !pwidth || !pheight)
    return(false);
  unsigned short* s_buf = new unsigned short[pwidth * pheight];
  float* mean = new float[pwidth * pheight];
  memset((void*)mean, 0, sizeof(float) * pwidth * pheight);
  float fraction = 1.0 / ( (col_no-col_begin) * (row_no - row_begin) * depth);
  for(int col=col_begin; col < col_no; ++col){
    for(int row=row_begin; row < row_no; ++row){
      for(int slice=0; slice < depth; ++slice){
	if(!data->readToShort(s_buf, col, row, slice, ch)){
	    cerr << "Unable to read short for " << col << "," << row << "," << slice << endl;
	    delete []s_buf;
	    delete []mean;
	    return(false);
	}
	toMean(mean, s_buf, pwidth * pheight, fraction);
      }
    }
  }
  // turn short to float using channel information..
  float* mptr = mean;
  for(mptr = mean; mptr < mean + (pwidth * pheight); ++mptr){
    *mptr = channels[ch].bias + channels[ch].scale * ((*mptr) / channels[ch].maxLevel);
  }
  par.setImage(mean);
  par.width = pwidth;
  par.height = pheight;
  par.x = par.y = 0;
  delete []s_buf;
  return(true);
}

void ImageBuilder::setPanelBias(f_parameter& par)
{
  short bias = 0;
  float scale = 1.0;
  int ch = -1;
  int row=-1;
  int col=-1;
  if(!par.param("b", bias) || !par.param("s", scale)){
    cerr << "ImageBuilder::setPanelBias, didn't obtain scale (s) or bias (b); aborting" << endl;
    return;
  }
  if(!par.param("row", row) || !par.param("col", col) || !par.param("ch", ch)){
    cerr << "ImageBuilder::setPanelBias, didn't obtain row (row), column (col) or channel (ch); aborting" << endl;
    return;
  }
  cout << "calling setPanelBias : " << ch << "," << col << "," << row << " : " << scale << "\t" << bias << endl;
  data->setPanelBias((unsigned int)ch, (unsigned int)col, (unsigned int)row, scale, bias);
}

void ImageBuilder::setFrameBackgroundPars(f_parameter& par)
{
  int wi = -1;
  int xm = 0;
  int ym = 0;
  float qnt = 0;
  bool bg_sub = true;
  if(!par.param("wi", wi) || !par.param("xm", xm) || !par.param("ym", ym)
     || !par.param("q", qnt)){
    cerr << "Please specify : wi (int) xm (int) ym (int) q (float)" << endl;
    return;
  }
  par.param("sub", bg_sub);
  data->setBackgroundPars((unsigned int)wi, xm, ym, qnt, bg_sub);
}

void ImageBuilder::addSlice(f_parameter& par)
{
  vector<unsigned int> ch;
  if(!par.param("ch", ',', ch) && !par.param("wi", ',', ch)){  // allow both ch= and wi=
    cerr << "ImageBuilder::addSlice please specify the channels ch=1,2,3 or wi=1,2,3 etc.." << endl;
    return;
  }
  // we better check we don't have any stupid channels..
  vector<unsigned int> wi;
  wi.reserve(ch.size());
  for(uint i=0; i < ch.size(); ++i){
    if(ch[i] < channels.size())
      wi.push_back(ch[i]);
  }
  if(!wi.size())
    return;
  unsigned int z;
  if(!par.param("z", z)){
    cerr << "ImageBuilcer::addSlice please specify the z position" << endl;
    return;
  }
  int x = 0;
  int y = 0;
  int w = data->pwidth();
  int h = data->pheight();
  // allow overwriting of these..
  par.param("x", x); par.param("y", y); par.param("w", w); par.param("h", h);
  if( (x < 0 || y < 0 || w <=0 || h <= 0) ){
    cerr << "ImageBuilder::addSlice please don't specify negative positions or dimensions" << endl;
    return;
  }
  x = (x >= data->pwidth()) ? 0 : x;
  y = (y >= data->pheight()) ? 0 : y;
  w = (x + w) > data->pwidth() ? (data->pwidth() - x) : w;
  h = (y + h) > data->pheight() ? (data->pheight() - y) : h;
  // and then get the floats using the appropriate function..
  bool use_cmap = true;  // whether or not we use the contribution map
  par.param("cmap", use_cmap);  // optional parameter
 
  float** images = getFloatImages(wi, z, x, y, w, h, use_cmap);
  for(uint i=0; i < wi.size(); ++i)
    toRGB(images[i], rgbData, channels[wi[i]], (w * h), x, y, w, h, false);
  deleteFloats(images, wi.size());
  setRGBImage(rgbData, data->pwidth(), data->pheight());
  
}

// Get an averaged imaged of a load of tiles, then find center by finding maximum
// rings.. 
void ImageBuilder::findCenter(f_parameter& par)
{
  int ch;
  if(!par.param("ch", ch))
    return;
  unsigned int wi = (unsigned int)ch;
  int row_begin = 0;
  int col_begin = 0;
  par.param("rb", row_begin);
  par.param("cb", col_begin);
  if(wi >= channels.size())
    return;
  
  int col_no, row_no, pwidth, pheight;
  col_no = row_no = pwidth = pheight = 0;
  int depth = data->sectionNo();
  data->stackDimensions(col_no, row_no, pwidth, pheight);
  p_parameter p_par = f_to_p_param(par, pwidth, pheight, 0, 0, 0);
  if(!p_mean_panel(p_par)){
    cerr << "ImageBuilder::findCenter p_mean_panel returned false" << endl;
    return;
  }
  if(!p_par.image()){
    cerr << "ImageBuilder::findCenter p_mean_panel returned true, but no image specified" << endl;
    return;
  }
  float* image = p_par.image();
  // we now need somehow to get a vector of radiuses from par (f_parameter)
  vector<int> radiuses;
  if(!par.param("r", ',', radiuses)){
    cerr << "ImageBuilder::findCenter unable to get radiuses (r=4,20,40,...) : " << endl;
    delete []image;
    return;
  }
  CenterFinder cfinder(image, pwidth, pheight, radiuses);
  vector<cf_circle> circles = cfinder.findCenter();
  // then draw the circles onto the image. Use white colour, or specify something.
  
}

// this function may destroy image. image should be set to the return value.
// it would probably be safer to take a pointer or reference to image, but.. 
float* ImageBuilder::modifyImage(int x, int y, int w, int h, float* image, f_parameter& par)
{
  // create a p_parameter, and then run a set of pipe functions in the order specified by
  // the optional pfunctions=func1,func2,func3 etc.
  // we may need to create an additional p_parameter for each fraction.. 
  vector<QString> functions;
  if(!par.param("pfunc", ',', functions))
    return(image);
  if(!functions.size())
    return(image);
  ps_function func = 0;
  for(uint i=0; i < functions.size(); ++i){
    f_parameter fpar = par;  // copies everything..
    fpar.setFunction(functions[i]);
    p_parameter ppar = f_to_p_param(fpar, w, h, x, y, image);
    if(!pipe_slice_functions.count( ppar.function )){
      cerr << "No function defined for : " << ppar.function << endl;
      return(image);
    }
    func = pipe_slice_functions[ ppar.function ];
    if(!(*this.*func)(ppar)){
      cerr << "ImageBuilder::modifyImage pipe function returned false returning" << endl;
      return(image);
    }
    image = ppar.image();
  }
  return(image);
}

p_parameter ImageBuilder::f_to_p_param(f_parameter& fp, int w, int h, int xo, int yo, float* img)
{
  map<string, float> pars;
  bool ok;
  map<QString, QString> fmap = fp.params();
  for(map<QString, QString>::iterator it=fmap.begin(); it != fmap.end(); ++it){
    float f = (*it).second.toFloat(&ok);
    if(ok)
      pars.insert(make_pair( toString((*it).first), f));
  }
  return(p_parameter(toString(fp.function()), img, w, h, xo, yo, pars));
}

string ImageBuilder::toString(QString qstr)
{
  string str(qstr.toAscii().constData());
  return(str);
}
