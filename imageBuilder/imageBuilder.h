#ifndef IMAGEBUILDER_H
#define IMAGEBUILDER_H

#include <vector>
#include <set>
#include <QString>
#include <map>
#include "p_parameter.h"
#include "f_parameter.h"

class FileSet;
class GLImage;
// Note that the distchooser and tabwidget in this case represent a kind of terrible
// hack. We should find a better way of handling them.
class DistChooser;
class TabWidget;

struct channel_info;
struct color_map;
struct backgroundPars;

class ImageBuilder {
 public:
  ImageBuilder(FileSet* fs, std::vector<channel_info>& ch);
  ~ImageBuilder();

  bool buildSlice(std::set<unsigned int> wi, unsigned int slice);
  bool buildProjection(std::vector<unsigned int> wi, unsigned int beg, unsigned int end);
  bool buildSpectrallySubtractedImage(std::set<unsigned int> wi, unsigned int slice);
  bool gaussianSlice(std::set<unsigned int> wi, unsigned int slice, unsigned int g_radius);

  bool addBackground(unsigned int wi, unsigned int slice, color_map cm);  
  bool subBackground(unsigned int wi, unsigned int slice);
  void paintSpectralResponse(std::set<unsigned int> wi, unsigned int slice);
  void paintSpectralBackground(std::set<unsigned int> wi, unsigned int slice, unsigned int ps_ch);

  bool addMCP(unsigned int wi, unsigned int beg, unsigned int end);

  bool setColor(unsigned int wi, float r, float g, float b);
  bool setScaleAndBias(unsigned int wi, float b, float s);
  bool setMaxLevel(unsigned int wi, float ml);
  bool setBackgroundPars(unsigned int wi, int cw, int ch, int cd, float qnt, bool sub);
  void resetRGB();
  void reportParameters();
  void exportTiff(QString fname);
  // both of the below take an RGB image. However, the setBigImage, uses the
  // glImage::setBigImage() function that should be more robust for different
  // image sizes. (Some bugs in the setRGBImage I believe).
  void setRGBImage(float* img, unsigned int width, unsigned int height);
  void setBigImage(float* img, int source_x, int source_y, int width, int height);

  // something to connect functions with.. 
  void pipe_slice(std::vector<p_parameter> pars, unsigned int wi, bool reset_rgb=false);
  void pipe_slice(std::vector<p_parameter> pars, channel_info& ch_info, bool reset_rgb=false);

  // connect to general parameters..
  void general_command(f_parameter& par);
  // obtain some useful parameters..
  void getStackStats(unsigned int wi, int xb, int yb, int zb, int s_width, int s_height, int s_depth);

 private:
  GLImage* image;
  FileSet* data;
  float* rgbData;
  unsigned short* sBuffer;
  unsigned int texture_size;
  std::vector<channel_info> channels;
  std::vector<DistChooser*> distributions; // specific functions update
  std::map<unsigned int, float> bg_spectrum;  // the expected spectral response of the background.
  TabWidget* distTabs;

  bool buildShortProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end);
  bool buildFloatProjection(float* p_buffer, unsigned int wi, int x, int y, int width, int height,
			    unsigned int beg, unsigned int end, bool use_cmap);
  unsigned short* buildShortMCPProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end);
  void maximize(unsigned short* a, unsigned short* b, unsigned long l);
  void maximize(float* a, float * b, unsigned long l);
  void maximize_by_cc(unsigned short* projection, unsigned short* ci, unsigned short* cp, unsigned short* cc, unsigned long l);
  void make_contrast(unsigned short* image_data, unsigned short* contrast_data, unsigned int w, unsigned int h);
  void sub_slice_bg(unsigned int wi, unsigned int slice, unsigned short* sb);
  void toRGB(unsigned short* sb, float* rgb, channel_info& ci, unsigned long l);
  void toRGB(float* fb, float* rgb, channel_info& ci, unsigned int long l);
  void toRGB(float* fb, float* rgb, channel_info& ci, unsigned long l, int x_off, int y_off, int width, int height, bool clear=false);
  unsigned short* background(unsigned int wi, unsigned int slice, bool& ok);
  unsigned short* background(unsigned short* sb, unsigned int w, unsigned int h, backgroundPars& bgp);
  // hack to overcome mismatched APIs. bugger
  void subImage(float* source, float* dest, 
		unsigned int s_width, unsigned int s_height,
		unsigned int s_x, unsigned int s_y,
		unsigned int d_width, unsigned int d_height
		);
  float* spectralBackgroundEstimate(float** images, std::vector<float> bgr, unsigned int im_no, unsigned int im_l);
  float** getFloatImages(std::vector<unsigned int> ch, unsigned int slice, bool use_cmap=false);
  float** getFloatImages(std::vector<unsigned int> ch, unsigned int slice, int x, int y, int w, int h, bool use_cmap=false);
  void deleteFloats(float** fl, unsigned int l);
  void toMean(float* mean, unsigned short* add, int l, float fraction);
  
  void drawCircle(float* image, int imWidth, int imHeight, int x, int y, int r);

  // a number of functions that can be connected in a pipe (p_slice, etc..).
  typedef bool (ImageBuilder::*ps_function)(p_parameter& par);
  std::map<std::string, ps_function> pipe_slice_functions;
  bool p_slice(p_parameter& par);
  bool p_gaussian(p_parameter& par);
  bool pl_gaussian(p_parameter& par);  // use a linear booleaen.. 
  bool p_sub_bg(p_parameter& par);
  bool p_sub_channel(p_parameter& par);
  bool p_mean_panel(p_parameter& par);

  // Functions that just take a f_parameter as a starting point
  typedef void (ImageBuilder::*g_function)(f_parameter& par);
  std::map<QString, g_function> general_functions;
  void build_fprojection(f_parameter& par); // usets readToFloat instead of readToShort
  void setPanelBias(f_parameter& par);
  void setFrameBackgroundPars(f_parameter& par);
  void addSlice(f_parameter& par);
  void findCenter(f_parameter& par);
  
  // This can be used by any of the above, but it's not one of the general_functions itself
  // note that the below functin may destroy image if it returns a different image.. 
  float* modifyImage(int x, int y, int w, int h, float* image, f_parameter& par); 

  p_parameter f_to_p_param(f_parameter& fp, int w, int h, int xo, int yo, float* img);
  std::string toString(QString qstr);
};

#endif
