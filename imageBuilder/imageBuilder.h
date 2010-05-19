#ifndef IMAGEBUILDER_H
#define IMAGEBUILDER_H

#include <vector>
#include <set>
#include <QString>

class FileSet;
class GLImage;


struct channel_info;
struct color_map;

class ImageBuilder {
 public:
  ImageBuilder(FileSet* fs, std::vector<channel_info>& ch);
  ~ImageBuilder();

  bool buildSlice(std::set<unsigned int> wi, unsigned int slice);
  bool buildProjection(std::vector<unsigned int> wi, unsigned int beg, unsigned int end);

  bool addBackground(unsigned int wi, unsigned int slice, color_map cm);  
  bool subBackground(unsigned int wi, unsigned int slice);

  bool addMCP(unsigned int wi, unsigned int beg, unsigned int end);

  bool setColor(unsigned int wi, float r, float g, float b);
  bool setScaleAndBias(unsigned int wi, float b, float s);
  bool setMaxLevel(unsigned int wi, float ml);
  bool setBackgroundPars(unsigned int wi, int cw, int ch, int cd, float qnt, bool sub);
  void resetRGB();
  void reportParameters();
  void exportTiff(QString fname);
  void setRGBImage(float* img, unsigned int width, unsigned int height);

 private:
  GLImage* image;
  FileSet* data;
  float* rgbData;
  unsigned short* sBuffer;
  unsigned int texture_size;
  std::vector<channel_info> channels;

  bool buildShortProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end);
  unsigned short* buildShortMCPProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end);
  void maximize(unsigned short* a, unsigned short* b, unsigned long l);
  void maximize_by_cc(unsigned short* projection, unsigned short* ci, unsigned short* cp, unsigned short* cc, unsigned long l);
  void make_contrast(unsigned short* image_data, unsigned short* contrast_data, unsigned int w, unsigned int h);
  void sub_slice_bg(unsigned int wi, unsigned int slice, unsigned short* sb);
  void toRGB(unsigned short* sb, float* rgb, channel_info& ci, unsigned long l);
  unsigned short* background(unsigned int wi, unsigned int slice, bool& ok);
  // hack to overcome mismatched APIs. bugger
  void subImage(float* source, float* dest, 
		unsigned int s_width, unsigned int s_height,
		unsigned int s_x, unsigned int s_y,
		unsigned int d_width, unsigned int d_height
		);


};

#endif
