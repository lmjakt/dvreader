//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

//Copyright Notice
/*
    eXintegrator integrated expression analysis system
    Copyright (C) 2004  Martin Jakt & Okada Mitsuhiro
  
    This file is part of the eXintegrator integrated expression analysis system. 
    eXintegrator is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version. 

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#include "jpgView.h"
#include <qwidget.h>
#include <qimage.h>
#include <qstring.h>
#include <qevent.h>
#include <qregion.h>
#include <qpainter.h>
#include <qfont.h>
#include <qcolor.h>
#include <qfontmetrics.h>
#include <QRectF>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>
#include <iostream>

using namespace std;

JpgView::JpgView(QWidget* parent,  const char* name) : 
  QWidget(parent, name)
{
  //moving = false;
  setBackgroundMode(Qt::NoBackground);
  xo = yo = 0;
  printZoomFactor = false;
  image = new QImage(256, 256, 32);
  image->fill(QColor(200, 0, 0).rgb());
//  image->fill(Qt::green.rgb());

  virtualScale = currentScale = 1;
  int w = (int)((float)image->width() * currentScale);
  int h = (int)((float)image->height() * currentScale);
  scaledImage = image->smoothScale(w, h);
  resize(scaledImage.size());
}

JpgView::~JpgView(){
  cout << "destroying myself, by bye my address is " << this << endl;;
}

void JpgView::setImage(QByteArray data){
  delete image;
  image = new QImage(data);  // which hopefully will work.. 
  int w = (int)((float)image->width() * currentScale);
  int h = (int)((float)image->height() * currentScale);
  scaledImage = image->smoothScale(w, h);
  //resize(scaledImage.size());
  update();
}

void JpgView::setImage(QImage* img){
  QImage* oldImage = image;
  image = img;
  delete oldImage;
  //  *image = img;
  int w = (int)((float)image->width() * currentScale);
  int h = (int)((float)image->height() * currentScale);
  scaledImage = image->smoothScale(w, h);
  //resize(scaledImage.size());
  update();
}

void JpgView::setImageFromFile(QString infile){
  image->load(infile);   // and hope that it works..
  int w = (int)((float)image->width() * currentScale);
  int h = (int)((float)image->height() * currentScale);
  scaledImage = image->smoothScale(w, h);
  setCaption(infile);
  update();
}

void JpgView::setLabel(QString label){
  labelText = label;
}

void JpgView::paintEvent(QPaintEvent* e){
  //if(!moving){
  QRect r = e->rect();
  //cout << "paint Event : r.x() " << r.x() << "  r.y() " << r.y() << "  r.width() " << r.width() << endl;
  //cout << "\t\tzoom Factor : " << currentScale << "  virtual Factor " << virtualScale << endl;
  int ixo =(int) (xo * (float)scaledImage.width());
  int iyo = (int) (yo * (float)scaledImage.height());

  QPainter p(this);
  
  p.drawImage(QRectF(r.x(), r.y(), r.width(), r.height()), scaledImage, QRectF(r.x()+ixo, r.y()+iyo, r.width(), r.height()));
//  bitBlt(this, r.x(), r.y(), &scaledImage, r.x()+ixo, r.y()+iyo, r.width(), r.height());
  QFont scaleFont("Arial", 18);
  scaleFont.setWeight(QFont::Bold);
  scaleFont.setStyleStrategy(QFont::PreferAntialias);
  if(printZoomFactor){    // then draw the virtual zoom factor in the thingy.. 
//    QPainter p(this);
    p.setFont(scaleFont);
    p.setPen(QPen(QColor(255, 255, 255), 1));
    p.drawText(e->rect(), Qt::AlignCenter, QString::number(virtualScale, 'f', 3));
  }
  if(labelText.length()){
//    QPainter p(this);
    p.setFont(scaleFont);
    p.setPen(QPen(QColor(255, 255, 255), 1));
    // now we have to work out how long the text will be .. QFontmetrics ?? 
    QFontMetrics metrics(scaleFont);
    QRect tr = metrics.boundingRect(labelText);
    tr.moveBottomRight(QPoint(width()-15, height()-15));
    p.drawText(tr.x(), tr.bottom(), labelText);;
  }    
  return;
  //}
  // if I'm moving just draw a box..
  //erase();
}

void JpgView::scale(float f){
  virtualScale = currentScale = f;
  int w = (int)((float)image->width() * currentScale);
  int h = (int)((float)image->height() * currentScale);
  scaledImage = image->smoothScale(w, h);
  //resize(scaledImage.size());
  update();
}

QSize JpgView::sizeHint() const {
  return(scaledImage.size());
}

void JpgView::mousePressEvent(QMouseEvent* e){
  x = lastX = e->x();
  y = lastY = e->y();
}

void JpgView::mouseMoveEvent(QMouseEvent* e){
  if(e->state() == Qt::LeftButton){
    xo += ((float)(lastX - e->x()))/(float)(scaledImage.width());
    yo += ((float)(lastY - e->y()))/(float)(scaledImage.height());
    // top the offsets
    xo = xo < 0 ? 0 : xo;
    yo = yo < 0 ? 0 : yo;
    xo = xo > 1 ? 1 : xo;
    yo = yo > 1 ? 1 : yo;
    update();
    //cout << "x " << e->x() << "  y " << e->y() << "  xo " << xo << "  yo " << yo << endl;
  }
  if(e->state() == (Qt::LeftButton + Qt::ControlModifier)){
    // change Virtual Scale, and print out.. 
    virtualScale += (float)(lastY - e->y())/(float)200;
    // make sure not below 0..
    virtualScale = virtualScale > 0.1 ? virtualScale : 0.1;
    // generate a paintevent and see if we can call the paintevent method.. 
    int w = 75;
    int h = 45;   // should be enough space for writing the the zoom factor in..
    printZoomFactor = true;
    repaint(x, y, w, h, false);
    //    QPaintEvent* pe = new QPaintEvent(QRegion(x, y, w, h), false);
    //paintEvent(pe);
  }
  if(e->state() == (Qt::RightButton + Qt::ControlModifier)){
    //moving = true;
    // we want to move by x and y...
    int dx = e->x() - lastX;
    int dy = e->y() - lastY;
    //int x = pos().x();
    //int y = pos().y();
    //move(x+dx, y+dy);
    //emit moveMe(dx, dy);
    //    move(e->pos());
    //update();
  }
  lastX = e->x();
  lastY = e->y();
}

void JpgView::mouseReleaseEvent(QMouseEvent* e){
  //moving = false;
  if(printZoomFactor){
    printZoomFactor = false;
    scale(virtualScale);
  }
}

  

