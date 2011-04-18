#ifndef IMAGEBUILDER_H
#define IMAGEBUILDER_H

#include <vector>
#include <set>
#include <QString>
#include <QObject>
#include <QColor>
#include <map>
#include "p_parameter.h"
#include "f_parameter.h"
#include "qnt_colors.h"
#include "stack_info.h"
#include "blob_set.h"
#include "Blob_mt_mapper_collection.h"
#include "../image/imageAnalyser.h"

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
class DistPlotter;
class ScatterPlotter;
class PerimeterWindow;
//class NearestNeighborMapper;
class NNMapper2;
class MaskMaker;
class CellCollection;

struct channel_info;
struct color_map;
struct backgroundPars;
struct blob;
struct CellOutlines;

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
  void resetOverlayData();
  void reportParameters();
  void exportTiff(QString fname);
  // both of the below take an RGB image. However, the setBigImage, uses the
  // glImage::setBigImage() function that should be more robust for different
  // image sizes. (Some bugs in the setRGBImage I believe).
  void setRGBImage(float* img, unsigned int width, unsigned int height);
  void setBigSubImage(float* img, int source_x, int source_y, int source_width, int source_height, 
		      int source_sub_x, int source_sub_y, int cp_width, int cp_height);
  void setBigSubOverlay(unsigned char* img, int source_x, int source_y, int source_width, int source_height,
			int source_sub_x, int source_sub_y, int cp_width, int cp_height);
  void setBigImage(float* img, int source_x, int source_y, int width, int height);
  void setBigOverlay(unsigned char* img, int source_x, int source_y, int width, int height);

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
  ImageAnalyser* imageAnalyser;   // this is shared with deltaviewer, and it shares the same fileSet as this class
  GLImage* image;
  FileSet* data;
  float* rgbData;
  unsigned char* overlayData;  // RGBA unsigned byte data for overlays etc.. 
  unsigned short* sBuffer;
  unsigned int texture_size;
  std::vector<channel_info> channels;
  std::vector<DistChooser*> distributions; // specific functions update
  std::map<unsigned int, float> bg_spectrum;  // the expected spectral response of the backg'''''''round.
  std::map<QString, ImStack*> imageStacks;    // can be assigned by functions, 
  std::multimap<ImStack*, Blob_mt_mapper*> mtMappers; // should be deleted if the ImStack is deleted.
  std::map<QString, std::vector<Blob_mt_mapper*> > mapper_collections; 
  ////////  I need to find some reasonable way of making sure that mapper_blob_set entries are erased
  ////////  when their corresponding Blob_mt_mapper objects are deleted (.. Not sure how to do this .. )
  //// WARNING, WARNING.. mapper_blob_sets should be removed to be replaced by mapper_sets below

  ///// Make sure to remove, or rather rewrite functions that rely in the mapper_blob_sets..
  //  std::map<QString, std::vector<blob_set> > mapper_blob_sets;  // QString refers to the entries in mapper_blob_set..
  //////////////////////////////|||||||||||
  std::map<QString, Blob_mt_mapper_collection*>  mapper_sets;  // Entry into this, rem
  ///////// WARNING.. entering a vector<Blob_mt_mapper*> into mapper_sets should remove it from mapper_collections.. 
  std::map<QString, BlobModel*> blobModels;
  std::map<QString, LinePlotter*> linePlotters;
  std::map<QString, DistPlotter*> distPlotters;
  std::map<QString, ScatterPlotter*> scatterPlotters;
  std::map<QString, QSemaphore*> mapper_collection_semaphores; // at some point make a reasonable data structure containing all of this.
  std::map<QString, PerimeterData> perimeterData;
  std::map<QString, PerimeterWindow*> perimeterWindows; 
  std::map<QString, NNMapper2*> neighborMappers;     // The key should also refer to keys in mapper_sets, This is assumed, but cannot be guaranteed
  std::map<QString, CellOutlines*> cellIdMasks;
  std::map<QString, CellCollection*> cellCollections;
  MaskMaker* maskMaker;

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
  void project_blobs(Blob_mt_mapper* bmapper, qnt_colors& qntColor, unsigned char alpha);
  void project_blob(blob* b, stack_info& pos, QColor& color);
  //  void project_blobs(Blob_mt_mapper* bmapper, float r=1.0, float g=1.0, float b=1.0);
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
  void set_par(f_parameter& par);
  void read_commands_from_file(f_parameter& par);
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
  void trainBlobSetModels(f_parameter& par);
  void map_perimeters(f_parameter& par);    // tries to find nuclei, or rather perimeters around things.
  void project_perimeters(f_parameter& par);
  void map_blobs_to_perimeters(f_parameter& par);
  void draw_blob_model(f_parameter& par);
  void compare_model(f_parameter& par);
  void plot_blob_pars(f_parameter& par);
  void plot_blob_set_dist(f_parameter& par);
  void mergeBlobs(f_parameter& par);
  void readBlobCriteria(f_parameter& par);
  //  void make_blob_model(f_parameter& par); // this is problematic; see .cpp for details
  void project_blob_collections(f_parameter& par);    // projects contents of collection of things.. 
  void project_blob_sets(f_parameter& par);
  void project_blob_ids(f_parameter& par);   // colour blobs by id (group or perimeter id). Assumes there is a mapping between collection_sets
  void make_cell_mask(f_parameter& par);
  void make_cells(f_parameter& par);
  void draw_cells(f_parameter& par);
  void draw_cell(f_parameter& par);
  void modifyCellPerimeter(f_parameter& par);
  void modifyCells(f_parameter& par);
  void reassign_blobs_cells(f_parameter& par);
  void export_cell_summary(f_parameter& par);
  void makeMaskMaker();
  void list_objects(f_parameter& par);       // list object, use parameters to change listing. 

  void setImageCenter(f_parameter& par);

  // These can be used by any of the above
  void overlayPoints(std::vector<int> points, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
  void overlayCellMask(unsigned short* mask, unsigned short border_increment, 
		       int xoff, int yoff, unsigned int width, unsigned int height,
		       std::vector<QColor>& colors, bool clear);
  std::vector<QColor> generateColors(unsigned char alpha);
  bool setVisible(int& x, int& y, int& width, int& height);
  bool editPerimeter(Perimeter per, QString perSource, int perId, bool clear);
  

  // note that the below functin may destroy image if it returns a different image.. 
  float* modifyImage(int x, int y, int w, int h, float* image, f_parameter& par);
  ImStack* imageStack(f_parameter& par);  // convenience function.. 
  ImStack* imageStack(uint wi, int x, int y, int z, uint w, uint h, uint d, bool use_cmap=true);

  p_parameter f_to_p_param(f_parameter& fp, int w, int h, int xo, int yo, float* img);
  std::string toString(QString qstr);

  QString generateGeneralHelp();
  QString generatePipeSliceHelp(QString& fname);
  QString generateGeneralFunctionHelp(QString& fname);

  //  float getBlobParameter(blob* b, QString parname);
  bool isPowerOfTwo(unsigned int i){
    return( (i != 0) && !(i & (i - 1)) );
  }

  private slots:
  void beginSegment(QPoint p, Qt::MouseButton button);
  void extendSegment(QPoint p, Qt::MouseButton button);
  void endSegment(QPoint p, Qt::MouseButton button);
  void maskMakerChanged();
  void modifyNextCell(int increment); // go backwards or forwards.. 
  void modifyPerimeter();    // 
};

#endif
