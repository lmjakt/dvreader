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

#ifndef COLORCHOOSER_H
#define COLORCHOOSER_H

#include <QWidget>
#include <QColor>
//#include <qpushbutton.h>
#include <QCheckBox>
#include <QString>
//#include <qlayout.h>
//Added by qt3to4:
#include <QPalette>
#include <QPushButton>
#include <QVBoxLayout>

class ColorChooser : public QWidget
{
    Q_OBJECT

    public :
    ColorChooser(QString label, int windex, int wlength, QColor c, QWidget* parent=0, const char* name=0);
    // the wavelength, the initial color, and the usual..
  ~ColorChooser(){
  }
  void color(float* r, float* g, float* b);  // return the color in floats.
  QColor color(){
      return(currentColor);
  }
  int wlength(){
    return(waveLength);
  }
  bool includeInMerger(){
      return(includeMergeBox->isChecked());
  }
  bool subtractColor(){
      return(subtractBox->isChecked());
  }
  void unCheckMergeButton(){
      includeMergeBox->setChecked(false);
  }
  QString displayLabel(){
      return(labelString);
  }

  private slots :
    void setColor();    // use a color dialog to set a slot..
  void objectFind();    // emit the signal with thingyes.. 
  void toggleColor(bool on);

  protected :
      QVBoxLayout* vbox;

  private :
    int waveLength;
  QString labelString;
  int waveIndex;
  QColor currentColor;
  QPalette palette;
  QColor constColor;
  QPushButton* button;
  QCheckBox* includeMergeBox;
  QCheckBox* subtractBox;
  
  signals :
    void colorChanged(int, float, float, float);   // first int is wavelength.. and the rest are nothings..
  void findObjects(int);                           // the wavelength.. 
  void checkSubtractions(bool);
};

#endif
