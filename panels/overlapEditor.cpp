#include "overlapEditor.h"
#include "frameRect.h"
#include <iostream>
#include <QColor>
#include <QKeyEvent>
#include <QPainter>

using namespace std;

OverlapEditor::OverlapEditor(bool invert, QWidget* parent)
  : QWidget(parent)
{
  fsInfo = 0;
  selectedFrame = 0;
  scale = 0.3;
  inverted=invert;
}

OverlapEditor::~OverlapEditor()
{
  deleteRects();
}

void OverlapEditor::setInfo(FileSetInfo* fsetInfo, float scale)
{
  // don't delete the old fsInfo, as this is most likely shared.
  deleteRects();
  vpanels.resize(0);
  fsInfo = fsetInfo;
  scale = scale;
  // use width and height to set the
  //  setSceneRect(QRectF( -0.1*(float)fsInfo->image_width(), -0.1*(float)fsInfo->image_height(), 1.2*(float)fsInfo->image_width(), 1.2*fsInfo->image_height()));
  //panel_pos.clear();
  makeRects(fsInfo);
}

QPoint OverlapEditor::offSet(float x, float y){
  QPoint p(0, 0);
  if(panels.count(x) && panels[x].count(y))
    return(panels[x][y]->offset());
  return(p);
}

void OverlapEditor::setOffset(float x, float y, QPoint p){
  if(panels.count(x) && panels[x].count(y)){
    panels[x][y]->setOffset(p);
    update();
  }
}


void OverlapEditor::mousePressEvent(QMouseEvent* e)
{
  setFocus();
  lastPoint = e->pos();
  selectedFrame = 0;
  QPoint pos = e->pos() - painterOffset;
  pos /= scale;
  for(uint i=0; i < vpanels.size(); ++i){
    if( vpanels[i]->contains(pos) && !selectedFrame ){
      vpanels[i]->select(true);
      if(vpanels[i]->isSelected())
	selectedFrame = vpanels[i];
    }else{
      vpanels[i]->select(false);
    }
  }
  update();
  if(selectedFrame){
    emit newFrameSelected(selectedFrame->finfo()->xpos, selectedFrame->finfo()->ypos);
  }
}

void OverlapEditor::mouseMoveEvent(QMouseEvent* e)
{
  QPoint thisPoint = e->pos();
  if(e->buttons() == Qt::RightButton){
    emit r_delta_y(lastPoint.y() - thisPoint.y());
    emit r_delta_x(thisPoint.x() - lastPoint.x());
    scale *= pow(2, ((float)(lastPoint.y() - thisPoint.y())/10.0));
    update();
  }
  if(e->buttons() == Qt::LeftButton){
    painterOffset += (thisPoint - lastPoint);
    update();
  }
  lastPoint = thisPoint;
}

void OverlapEditor::mouseReleaseEvent(QMouseEvent* e)
{
  
}

void OverlapEditor::keyPressEvent(QKeyEvent* event)
{
  if(!selectedFrame)
    return;
  int c = event->key();
  switch(c){
  case Qt::Key_Up:
    selectedFrame->moveBy(QPoint(0, -1));
    break;
  case Qt::Key_Down:
    selectedFrame->moveBy(QPoint(0, 1));
    break;
  case Qt::Key_Left:
    selectedFrame->moveBy(QPoint(-1, 0));
    break;
  case Qt::Key_Right:
    selectedFrame->moveBy(QPoint(1, 0));
    break;
  default:
    ;
  }
  if(c == Qt::Key_Up || c == Qt::Key_Down 
     || c == Qt::Key_Left || c == Qt::Key_Right){
    update();
    emit offSetChanged(selectedFrame->offset());
  }
}

void OverlapEditor::paintEvent(QPaintEvent* e)
{
  QPainter p(this);
  p.translate(painterOffset);
  p.scale(scale, scale);
  for(uint i=0; i < vpanels.size(); ++i)
    vpanels[i]->paint(&p, height(), scale);
}


void OverlapEditor::deleteRects(){

  for(map<float, map<float, FrameRect*> >::iterator ot=panels.begin(); 
      ot != panels.end(); ot++){
    for(map<float, FrameRect*>::iterator it = (*ot).second.begin();
	it != (*ot).second.end(); ++it){
      delete( (*it).second );
    }
    (*ot).second.clear();
  }
  panels.clear();
  //panel_pos.clear();
}

void OverlapEditor::makeRects(FileSetInfo* fsi){
  for(map<float, map<float, FrameInfo*> >::iterator ot=fsi->stacks.begin();
      ot != fsi->stacks.end(); ++ot){
    for(map<float, FrameInfo*>::iterator it=(*ot).second.begin();
	it != (*ot).second.end(); ++it){
      FrameRect* rect = new FrameRect((*it).second, inverted);
      vpanels.push_back(rect);
      //      QGraphicsRectItem* rect = new QGraphicsRectItem( (*it).second->xp, (*it).second->yp,
      //						       (*it).second->width, (*it).second->height);
      //panel_pos.insert(make_pair( rect, a_pos( (*it).second->xp, (*it).second->yp) ));
      panels[(*ot).first][(*it).first] = rect;
      cout << "added new rect at : " << (*it).second->xp << "," << (*it).second->yp
	   << "  : " << (*it).second->width << "," << (*it).second->height << endl;
    }
  }
  cout << "\n\n\n\n" << endl;
}
