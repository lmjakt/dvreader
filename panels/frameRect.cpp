#include "frameRect.h"
#include "fileSetInfo.h"  // also contains FrameInfo definition.
#include <QPainter>
#include <QWidget>
#include <QGraphicsItem>
#include <QColor>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QPoint>
#include <QString>
#include <QTextStream>
#include <iostream>

using namespace std;

FrameRect::FrameRect(FrameInfo* finfo, bool invert)
{
  frame = finfo;
  inverted = invert;
  selected = false;
  offSet = QPoint(0, 0);
}

FrameRect::~FrameRect()
{
  // Don't delete the frame; it belongs to someone else.
}

void FrameRect::paint(QPainter* painter, int h, float scale)
{
  painter->save();

  if(selected){
    painter->setPen(QPen(QColor(0, 0, 255), 0));
    painter->setBrush(QColor(0, 0, 255, 50));
  }else{
    painter->setPen(QPen(QColor(55, 0, 0), 0));
    painter->setBrush(QColor(150, 150, 150, 20)); 
  }
  
  if(!inverted){
    rect.setRect(offSet.x() + frame->xp, 
		 offSet.y() + frame->yp, 
		 frame->width, frame->height);
  }else{
    rect.setRect(offSet.x() + frame->xp,
		 h/scale-(frame->yp + frame->height + offSet.y()),
		 frame->width, frame->height);
  }

  painter->drawRect(rect);
  QRect textRect(rect);
  QString line1;
  QString line2;
  QTextStream ts1(&line1);
  QTextStream ts2(&line2);
  ts1 << "Position\t" << frame->xp << "," << frame->yp;
  ts2 << "Offset\t " << offSet.x() << "," << offSet.y(); 
  painter->setFont(QFont("Arial", 30));
  painter->drawText(rect, Qt::AlignCenter, line1, &textRect);
  textRect.moveTop( textRect.bottom() );
  painter->drawText(textRect, Qt::AlignLeft, line2);
  painter->restore();
}

bool FrameRect::contains(QPoint p){
  return(rect.contains(p));
}

void FrameRect::toggleSelection(){
  selected = !selected;
}

// if already selected, deselect, otherwise set 
// to whatever s is. 
void FrameRect::select(bool s){
  selected = s;
  //  selected = selected ? false : s;
}

void FrameRect::moveBy(QPoint p){
  if(!inverted){
    offSet += p;
    return;
  }
  offSet.rx() += p.x();
  offSet.ry() -= p.y();
}

bool FrameRect::isSelected(){
  return(selected);
}

QPoint FrameRect::pos(int height){
  if(!inverted)
    return( QPoint(frame->xp, frame->yp) + offSet );
  return(QPoint( frame->xp + offSet.x(),
		 height - (frame->yp + offSet.y())));
}

QPoint FrameRect::offset(){
  return(offSet);
}

void FrameRect::setOffset(QPoint p){
  offSet = p;
}
