#include "overlapEditor.h"
#include <iostream>
#include <QGraphicsRectItem>
#include <QColor>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

using namespace std;

OverlapEditor::OverlapEditor(QObject* parent)
  : QGraphicsScene(parent)
{
  fsInfo = 0;
  currentFrame = 0;
  scaleIsChanging = false;
}

OverlapEditor::~OverlapEditor()
{
  deleteRects();
}

void OverlapEditor::setInfo(FileSetInfo* fsetInfo)
{
  // don't delete the old fsInfo, as this is most likely shared.
  deleteRects();
  fsInfo = fsetInfo;
  // use width and height to set the
  setSceneRect(QRectF( -0.1*(float)fsInfo->image_width(), -0.1*(float)fsInfo->image_height(), 1.2*(float)fsInfo->image_width(), 1.2*fsInfo->image_height()));
  panel_pos.clear();
  makeRects(fsInfo);
}

void OverlapEditor::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  QGraphicsScene::mousePressEvent(mouseEvent);
  currentFrame = (QGraphicsRectItem*)mouseGrabberItem();
}

void OverlapEditor::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  if(mouseEvent->buttons() == Qt::RightButton && !scaleIsChanging){
    scaleIsChanging = true;
    QPoint lastPoint = mouseEvent->lastScreenPos();
    QPoint thisPoint = mouseEvent->screenPos();
    emit r_delta_y(lastPoint.y() - thisPoint.y());
    emit r_delta_x(thisPoint.x() - lastPoint.x());
    scaleIsChanging = false;
  }
  QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void OverlapEditor::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
  QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void OverlapEditor::keyPressEvent(QKeyEvent* event)
{
  if(!currentFrame)
    return;
  switch(event->key()){
  case Qt::Key_Up:
    currentFrame->moveBy(0, 1);
    break;
  case Qt::Key_Down:
    currentFrame->moveBy(0, -1);
    break;
  case Qt::Key_Left:
    currentFrame->moveBy(-1, 0);
    break;
  case Qt::Key_Right:
    currentFrame->moveBy(1, 0);
    break;
  default:
    ;
  }
}

void OverlapEditor::deleteRects(){
  for(map<float, map<float, QGraphicsRectItem*> >::iterator ot=panels.begin(); 
      ot != panels.end(); ot++){
    for(map<float, QGraphicsRectItem*>::iterator it = (*ot).second.begin();
	it != (*ot).second.end(); ++it){
      delete( (*it).second );
    }
    (*ot).second.clear();
  }
  panels.clear();
  panel_pos.clear();
}

void OverlapEditor::makeRects(FileSetInfo* fsi){
  for(map<float, map<float, FrameInfo*> >::iterator ot=fsi->stacks.begin();
      ot != fsi->stacks.end(); ++ot){
    for(map<float, FrameInfo*>::iterator it=(*ot).second.begin();
	it != (*ot).second.end(); ++it){
      QGraphicsRectItem* rect = new QGraphicsRectItem( (*it).second->xp, (*it).second->yp,
						       (*it).second->width, (*it).second->height);
      panel_pos.insert(make_pair( rect, a_pos( (*it).second->xp, (*it).second->yp) ));
      rect->setBrush(Qt::NoBrush);
      //      rect->setFlag(QGraphicsItem::ItemIsMovable, true);
      rect->setFlag(QGraphicsItem::ItemIsSelectable, true);
      //rect->setBrush(QColor(140, 140, 175));
      panels[(*ot).first][(*it).first] = rect;
      addItem(rect);
      cout << "added new rect at : " << (*it).second->xp << "," << (*it).second->yp
	   << "  : " << (*it).second->width << "," << (*it).second->height << endl;
    }
  }
  cout << "\n\n\n\n" << endl;
}
