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

#include "lSpinBox.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <iostream>
#include <qcolor.h>
#include <qstring.h>

LSpinBox::LSpinBox(int minValue, int maxValue, int step, QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  label = new QLabel(name, this, name);
  label->setPaletteBackgroundColor(QColor(255, 255, 255));
  label->setMargin(2);
  label->setAlignment(Qt::AlignLeft);
  label->setIndent(1);

  QString numString;
  numString.setNum(maxValue + 1);
  sizeLabel = new QLabel(numString, this, "sizeLabel");
  sizeLabel->setPaletteBackgroundColor(QColor(255, 255, 255));
  sizeLabel->setMargin(1);
  sizeLabel->setAlignment(Qt::AlignLeft);
  sizeLabel->setIndent(2);

  spin = new QSpinBox(minValue, maxValue, step, this, name);
  connect(spin, SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)) );
  QVBoxLayout* lout = new QVBoxLayout(this);
  lout->addWidget(label);
  lout->addWidget(sizeLabel);
  lout->addWidget(spin);
}

int LSpinBox::value(){
  return(spin->value());
}

void LSpinBox::setRange(int min, int max){
  QString numString;
  numString.setNum(max + 1);
  sizeLabel->setText(numString);
  spin->setRange(min, max);
}

void LSpinBox::setMaxValue(int max){
  QString numString;
  numString.setNum(max + 1);
  sizeLabel->setText(numString);
  spin->setMaxValue(max);
}

void LSpinBox::setValue(int v){
  spin->setValue(v);
}
