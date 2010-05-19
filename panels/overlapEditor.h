#ifndef OVERLAPEDITOR_H
#define OVERLAPEDITOR_H

#include <QWidget>
#include <QPoint>
#include <map>
#include "fileSetInfo.h"
#include "../datastructs/a_pos.h"

class FrameRect;


class OverlapEditor : public QWidget
{

  Q_OBJECT

    public:
  OverlapEditor(bool invert, QWidget* parent=0);
  ~OverlapEditor();
  void setInfo(FileSetInfo* fsetInfo, float scale);
  QPoint offSet(float x, float y);
  void setOffset(float x, float y, QPoint p);
  
 signals:
  void r_delta_y(int);
  void r_delta_x(int);
  void newFrameSelected(float, float);
  void offSetChanged(QPoint);

 protected:
  void mousePressEvent(QMouseEvent* mouseEvent);
  void mouseMoveEvent(QMouseEvent* mouseEvent);
  void mouseReleaseEvent(QMouseEvent* mouseEvent);
  void keyPressEvent(QKeyEvent* event);
  void paintEvent(QPaintEvent* event);

 private:
  void deleteRects();
  void makeRects(FileSetInfo* fsi);

  FileSetInfo* fsInfo;
  std::map< float, std::map<float, FrameRect*> > panels;
  std::vector< FrameRect* > vpanels;  // for easy accessx
  bool inverted;
  //std::map< FrameRect*, a_pos> panel_pos;
  //  std::map< float, std::map<float, QGraphicsRectItem*> > panels;
  //std::map< QGraphicsRectItem*, a_pos> panel_pos;
  FrameRect* selectedFrame;
  QPoint lastPoint;
  //  QGraphicsRectItem* currentFrame;
  float scale;
  QPoint painterOffset;
};

#endif
