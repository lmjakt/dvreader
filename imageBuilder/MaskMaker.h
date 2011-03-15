#ifndef MASKMAKER_H
#define MASKMAKER_H

#include <QPoint>
#include <vector>

// the mask in detail is kept as an unsigned RGBA array.
// that is a bit wasteful, but it can be directly applied
// as a glImage overlay at any point without converting the
// two.

class MaskMaker
{
 public:
  MaskMaker(QPoint mask_pos, int w, int h);
  ~MaskMaker();

  void setPerimeter(std::vector<QPoint> p, bool global=true);
  void startSegment(QPoint p, bool global=true);
  void addSegmentPoint(QPoint p, bool global=true);
  void endSegment(QPoint p, bool global=true);
  void setMaskColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

  unsigned char* maskImage(QPoint& pos, int& w, int& h);    // 

  const static unsigned char PERIMETER = 1;
  const static unsigned char SEGMENT = 2;
  const static unsigned char BEGIN = 4;
  const static unsigned char END = 8;

 private:
  std::vector<QPoint> perimeter;
  std::vector<std::vector<QPoint> > borderSegments;
  std::vector<QPoint> currentSegment;         // points specified by the user
  std::vector<QPoint> currentSegmentPoints;   // includes intervening points.
  
  unsigned char* mask;              // a binary orred mask
  unsigned char* maskPicture;       // pictures of that mask.
  unsigned char* maskPictureBuffer; // temporary picture of the mask.

  QPoint origin;
  unsigned int width;
  unsigned int height;

  unsigned char red, green, blue, alpha;

  std::vector<QPoint> drawLine(QPoint& p1, QPoint& p2, unsigned char* m);
  void drawPoints(std::vector<QPoint>& points, unsigned char* m);
  void setMask(std::vector<QPoint>& points, unsigned char v, bool setEnds);
  bool checkSegment(std::vector<QPoint>& points);

  void globalToLocal(QPoint& p){
    p -= origin;
  }
  void localToGlobal(QPoint& p){
    p += origin;
  }
  bool checkPoint(QPoint& p){
    return( p.x() >= 0 && p.x() < (int)width && p.y() >= 0 && p.y() < (int)height);
  }
  unsigned char maskValue(QPoint& p){  // no error checking
    return( mask[p.y() * width + p.x() ]);
  }
};

#endif
