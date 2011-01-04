#ifndef IMAGEBUILDER_H
#define IMAGEBUILDER_H

#include <vector>
#include <set>
#include <QString>
#include <QObject>
#include <map>
#include "p_parameter.h"
#include "f_parameter.h"

class FileSet;
class GLImage;
// Note that the distchooser and tabwidget in this case represent a kind of terrible
// hack. We should find a better way of handling them.
class DistChooser;
class TabWidget;
class ImStack;
class Blob_mt_mapper;
class QSemaphore;
class BlobModel;
class LinePlotter;

struct channel_info;
struct color_map;
struct backgroundPars;

class ImageBuilder : public QObject 
{
  Q_OBJECT
 public:
  ImageBuilder(FileSet* fs, std::vector<channel_info>& ch, QObject* parent=0);
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

  QString help(std::vector<QString>& words);

 signals:
  void stackCreated(QString);
  void stackDeleted(QString);
  void displayMessage(const char*);
 private:
  GLImage* image;
  FileSet* data;
  float* rgbData;
  unsigned short* sBuffer;
  unsigned int texture_size;
  std::vector<channel_info> channels;
  std::vector<DistChooser*> distributions; // specific functions update
  std::map<unsigned int, float> bg_spectrum;  // the expected spectral response of the background.
  std::map<QString, ImStack*> imageStacks;    // can be assigned by functions, 
  std::multimap<ImStack*, Blob_mt_mapper*> mtMappers; // should be deleted if the ImStack is deleted.
  std::map<QString, std::vector<Blob_mt_mapper*> > mapper_collections; 
  std::map<QString, BlobModel*> blobModels;
  std::map<QString, LinePlotter*> linePlotters;
  std::map<QString, QSemaphore*> mapper_collection_semaphores; // at some point make a reasonable data structure containing all of this.

  TabWidget* distTabs;

  bool buildShortProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end);
  bool buildFloatProjection(float* p_buffer, unsigned int wi, int x, int y, int width, int height,
			    unsigned int beg, unsigned int end, bool use_cmap);
  unsigned short* buildShortMCPProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end);
  float* projectStack_f(float* stack, unsigned int w, unsigned int h, unsigned int d);
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
  void project_blobs(Blob_mt_mapper* bmapper, float r=1.0, float g=1.0, float b=1.0);
  // imStack related stuff ..
  // if add is true add to imageStacks, otherwise delete
  // emit the correct signals..
  void updateStacks(QString& name, ImStack* stack, bool add);
  void warn(const char* message);
  void warn(QString& message);

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
  void build_fgprojection(f_parameter& par);  // builds a projection from a blurred stack.
  void setPanelBias(f_parameter& par);
  void setFrameBackgroundPars(f_parameter& par);
  void addSlice(f_parameter& par);
  void blurStack(f_parameter& par);
  void deBlurStack(f_parameter& par);
  void subStack(f_parameter& par);
  void subStackBackground(f_parameter& par);
  void findCenter(f_parameter& par);
  void loopStack(f_parameter& par);
  void loopXZ(f_parameter& par);
  void stack_xzSlice(f_parameter& par);
  void stack_yzSlice(f_parameter& par);
  void stack_project(f_parameter& par);
  void setStackPar(f_parameter& par);
  //  void stackSlice(f_parameter& par);
  void stack_map_blobs(f_parameter& par);  // a testing function. to see if the mt_blob_mapper works.. 
  void map_blobs(f_parameter& par);        // makes stacks to map. Splits the problem into smaller parts.
  void draw_blob_model(f_parameter& par);
  //  void make_blob_model(f_parameter& par); // this is problematic; see .cpp for details
  void project_blob_collections(f_parameter& par);    // projects contents of collection of things.. 
  void list_objects(f_parameter& par);       // list object, use parameters to change listing. 
  // This can be used by any of the above, but it's not one of the general_functions itself
  // note that the below functin may destroy image if it returns a different image.. 
  float* modifyImage(int x, int y, int w, int h, float* image, f_parameter& par); 
  ImStack* imageStack(f_parameter& par);  // convenience function.. 
  ImStack* imageStack(uint wi, int x, int y, int z, uint w, uint h, uint d, bool use_cmap=true);

  p_parameter f_to_p_param(f_parameter& fp, int w, int h, int xo, int yo, float* img);
  std::string toString(QString qstr);

  QString generateGeneralHelp();
  QString generatePipeSliceHelp(QString& fname);
  QString generateGeneralFunctionHelp(QString& fname);

};

#endif
