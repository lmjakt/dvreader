#ifndef OUTLINETRACER_H
#define OUTLINETRACER_H

#include <vector>

class OutlineTracer {
 public:
  OutlineTracer(unsigned char* msk, int x, int y, unsigned int m_width, unsigned int m_height, unsigned int g_width, bool destructive);
  OutlineTracer(unsigned char* msk, unsigned int m_width, unsigned int m_height, bool destructive);
  ~OutlineTracer();

  std::vector<int> traceOutline(unsigned char border);

 private:
  unsigned char* mask;
  unsigned int mask_width, mask_height;
  int mask_x, mask_y;
  unsigned int global_width;
  bool deleteMask;
};

#endif
