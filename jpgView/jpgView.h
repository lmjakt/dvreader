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

#ifndef JPGVIEW_H
#define JPGVIEW_H

#include <qwidget.h>
#include <qsize.h>
#include <qimage.h>
#include <qstring.h>
#include <qrect.h>
#include <q3cstring.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>

using namespace std;

class JpgView : public QWidget
{
  Q_OBJECT
    public :
    JpgView(QWidget* parent=0, const char* name=0);
  ~JpgView();
  QSize sizeHint() const;
  QImage* imagePointer(){
    return(image);
  }
  QSize viewSize(){
    return(scaledImage.size());
  }

  public slots :
    void setImage(QByteArray data);
  void setImage(QImage* img);
  void scale(float f);      // a float between 0 and 1.. if not than set to 1. 
  void setImageFromFile(QString infile);
  void setLabel(QString label);
  
  private :
    void paintEvent(QPaintEvent* e);
 
  virtual void mouseMoveEvent(QMouseEvent* e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent(QMouseEvent* e);
  
  protected :
  // how to select the drawing area..
  int lastX;
  int lastY;
  float yo;
  float xo;   // the drawing offsets.. -- only draw the size of the widget, and offset according to these values.. 
  float currentScale;
  float virtualScale;  // for keeping track of stuff...
  //bool moving;         // don't draw if moving..              // have offsets between 0 and 1 as fractional offsets.. 
  bool printZoomFactor;    // paint event just for painting zoom factor or for other purpose?? 
  QImage* image;
  QImage scaledImage;
  QString labelText;     // something to put the labe in for.. 

  int x, y;                // set at a mouse press event.. if we want to keep.. 


  signals : 
    //void moveMe(int, int);   // move me please.. 
};

#endif
