#ifndef IMAGEBUILDER_H
#define IMAGEBUILDER_H

#include <vector>
#include <set>

class FileSet;
class GLImage;


struct channel_info;

class ImageBuilder {
 public:
  ImageBuilder(FileSet* fs, std::vector<channel_info>& ch);
  ~ImageBuilder();

  bool buildSlice(std::set<unsigned int> wi, unsigned int slice);
  bool buildProjection(std::vector<unsigned int> wi, unsigned int beg, unsigned int end);

  bool setColor(int wi, float r, float g, float b);
  bool setScaleAndBias(int wi, float b, float s);
  bool setMaxLevel(int wi, float ml);
  void reportParameters();

 private:
  GLImage* image;
  FileSet* data;
  float* rgbData;
  unsigned short* sBuffer;
  unsigned int texture_size;
  std::vector<channel_info> channels;

  bool buildShortProjection(unsigned short* p_buffer, unsigned int wi, unsigned int beg, unsigned int end);
  void maximize(unsigned short* a, unsigned short* b, unsigned long l);
  void toRGB(unsigned short* sb, float* rgb, channel_info& ci, unsigned long l);
  // hack to overcome mismatched APIs. bugger
  void setRGBImage(float* img, unsigned int width, unsigned int height);
  void subImage(float* source, float* dest, 
		unsigned int s_width, unsigned int s_height,
		unsigned int s_x, unsigned int s_y,
		unsigned int d_width, unsigned int d_height
		);


};

#endif
