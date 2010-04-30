#ifndef OVERLAPEDITOR_H
#define OVERLAPEDITOR_H

#include <QGraphicsScene>
#include <map>
#include "fileSetInfo.h"
#include "../datastructs/a_pos.h"

class OverlapEditor : public QGraphicsScene
{

  Q_OBJECT

    public:
  OverlapEditor(QObject* parent=0);
  ~OverlapEditor();
  void setInfo(FileSetInfo* fsetInfo);

 signals:
  void r_delta_y(int);
  void r_delta_x(int);

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
  void mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent);
  void keyPressEvent(QKeyEvent* event);

 private:
  void deleteRects();
  void makeRects(FileSetInfo* fsi);
  FileSetInfo* fsInfo;
  std::map< float, std::map<float, QGraphicsRectItem*> > panels;
  std::map< QGraphicsRectItem*, a_pos> panel_pos;
  QGraphicsRectItem* currentFrame;
  bool scaleIsChanging;

};

#endif
