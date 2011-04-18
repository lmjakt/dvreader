#ifndef MASKMAKER_H
#define MASKMAKER_H

#include <QPoint>
#include <QObject>
#include <QString>
#include <vector>
#include <set>

// the mask in detail is kept as an unsigned RGBA array.
// that is a bit wasteful, but it can be directly applied
// as a glImage overlay at any point without converting the
// two.

class QKeyEvent;


class MaskMaker : public QObject
{
  Q_OBJECT
 public:
  MaskMaker(QPoint mask_pos, int w, int h, QObject* parent=0);
  ~MaskMaker();
  
  void newMask(QPoint mask_pos, int w, int h);
  void setPerimeter(std::vector<QPoint> p, int id, QString source, bool global=true);
  void setPerimeter(std::vector<QPoint> p, bool global=true, bool resetIDs=true);
  void setCellSource(QString source, int id);
  int per_id();
  QString per_source();
  
  std::vector<QPoint> getPerimeter(int& id, QString& source, bool global=true);
  void startSegment(QPoint p, bool global=true);
  void addSegmentPoint(QPoint p, bool global=true);
  void endSegment(QPoint p, bool global=true);
  void setMaskColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

  unsigned char* maskImage(QPoint& pos, int& w, int& h);    // 

  const static unsigned char PERIMETER = 1;
  const static unsigned char SEGMENT = 2;
  const static unsigned char BEGIN = 4;
  const static unsigned char END = 8;

 public slots:
  void keyPressed(QKeyEvent* e);

 signals:
  void maskChanged();
  void perimeterModified();
  void increment(int);
  
 private:
  std::vector<QPoint> perimeter;
  int perimeter_id;
  QString perimeter_source;
  std::vector<QPoint> original_perimeter;
  std::vector<std::vector<QPoint> > borderSegments;
  std::vector<QPoint> currentSegment;         // points specified by the user
  std::vector<QPoint> currentSegmentPoints;   // includes intervening points.
  
  unsigned char* mask;              // a binary orred mask
  unsigned char* maskPicture;       // pictures of that mask.
  /// REMOVE maskPictureBuffer and write a drawFromMask() function instead. Do all temporary drawing to picture and
  /// finish by calling drawFromMask.
  //unsigned char* maskPictureBuffer; // temporary picture of the mask.

  QPoint origin;
  unsigned int width;
  unsigned int height;
  
  unsigned char red, green, blue, alpha;

  std::vector<QPoint> drawLine(QPoint& p1, QPoint& p2, unsigned char* m, bool filled);
  void drawPoints(std::vector<QPoint>& points, unsigned char* m);
  void setMask();
  void setMask(std::vector<QPoint>& points, unsigned char v, bool setEnds);
  bool checkSegment(std::vector<QPoint>& points);
  void mergeSegments();
  void checkSegmentDirection(std::vector<QPoint>& points);  // reverses if anti-clockwise.

  bool addPoints(std::vector<QPoint>& points, std::vector<QPoint>& segment, std::set<int>& point_set);
  bool segmentFrom(QPoint& p, std::vector<QPoint>& points);
  bool perimeterIndexAt(QPoint& p, unsigned int& index);
  void resetPerimeter(std::vector<QPoint>& points);  // resets all masks and everything. be careful..
  void drawFromMask();

  void removeDuplicates(std::vector<QPoint>& points);  // hack to remove bug I don't understand.

  void paintPoint(QPoint& p, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

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
  void setRGBA(unsigned char* m, unsigned char r, unsigned char g, unsigned char b, unsigned char a){
    m[0] = r;
    m[1] = g;
    m[2] = b;
    m[3] = a;
  }
};

#endif
