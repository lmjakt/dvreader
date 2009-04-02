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

#ifndef LSPINBOX_H
#define LSPINBOX_H

#include <qspinbox.h>
#include <qwidget.h>
#include <qlabel.h>

class LSpinBox : public QWidget         // a spin box with associtaed label.. 
{
  Q_OBJECT

    public :
    LSpinBox(int minValue, int maxValue, int step, QWidget* parent, const char* name);
  int value();           // return the spin value. 

  public slots :
    void setRange(int min, int max);
  void setMaxValue(int max);
  void setValue(int v);
    
    signals :
    void valueChanged(int);
  

  private :
    QLabel* label;
  QLabel* sizeLabel;
  QSpinBox* spin;
};

#endif
