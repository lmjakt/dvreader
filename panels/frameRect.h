#ifndef FRAMERECT_H
#define FRAMERECT_H

#include <QRect>
#include <QPoint>

struct FrameInfo;
class QPainter;

class FrameRect
{
    public:
  FrameRect(FrameInfo* finfo, bool invert);
  ~FrameRect();
  
  void paint(QPainter* painter, int h, float scale);
  bool contains(QPoint p);
  void toggleSelection();
  void select(bool s);
  void moveBy(QPoint p);
  bool isSelected();
  QPoint pos(int height);
  QPoint offset();
  void setOffset(QPoint p);
  const FrameInfo* finfo(){
    return(frame);
  }

 private:
  FrameInfo* frame;
  QRect rect;
  QPoint offSet;
  bool selected;
  bool inverted;
};

#endif
