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

#include "colorChooser.h"
#include <QLabel>
//#include <qlayout.h>
#include <QColorDialog>
//#include <qpushbutton.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <iostream>

using namespace std;

ColorChooser::ColorChooser(QString label, int windex, int wlength, QColor c, QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  currentColor = c;
  constColor = c;
  waveLength = wlength;
  waveIndex = windex;

  palette = QPalette();
  palette.setColor(QPalette::Button, currentColor);

  QString waveString;
  waveString.setNum(waveLength);
//  QLabel* wLabel = new QLabel(waveString, this, "wLabel");
  labelString = label;
  QLabel* wLabel = new QLabel(label, this, "wLabel");
  button = new QPushButton(this);
//  button = new QPushButton(this, "button");

  button->setPalette(palette);
//  button->setPaletteBackgroundColor(currentColor);

  button->setFixedWidth(30);
  button->setFixedHeight(30);
  connect(button, SIGNAL(clicked()), this, SLOT(setColor()) );

  // and let's have a button for finding objects...
//  QPushButton* objectButton = new QPushButton("objects", this, "objectButton");
//  connect(objectButton, SIGNAL(clicked()), this, SLOT(objectFind()) );

  includeMergeBox = new QCheckBox(this, "includeMergeBox");

  QCheckBox* includeColor = new QCheckBox(this, "includeColor");
  includeColor->setChecked(true);
  connect(includeColor, SIGNAL(toggled(bool)), this, SLOT(toggleColor(bool)) );
  
  subtractBox = new QCheckBox(this, "subtractBox");
  subtractBox->setChecked(false);
  connect(subtractBox, SIGNAL(toggled(bool)), this, SIGNAL(checkSubtractions(bool)) );  // which actually ignores the thingy

  vbox = new QVBoxLayout(this);

  QHBoxLayout* box = new QHBoxLayout();
  //  QHBoxLayout* box = new QHBoxLayout(this);
  vbox->addLayout(box);
  box->setSpacing(0);
  box->setMargin(0);
  box->setContentsMargins(0, 0, 0, 0);
  box->addWidget(wLabel);
  box->addStretch();
//  box->addWidget(objectButton);
  box->addWidget(includeMergeBox);
  box->addSpacing(5);
  box->addWidget(button);
  box->addWidget(includeColor);
  box->addWidget(subtractBox);
}

void ColorChooser::setColor(){
  QColor c = QColorDialog::getColor(currentColor);
  int ri, gi, bi;
  float r, g, b;
  
  c.rgb(&ri, &gi, &bi);
  //  c.getRgb(&ri, &gi, &bi);
  r = (float)ri/255.0;
  g = (float)gi/255.0;
  b = (float)bi/255.0;
  currentColor = c;
  constColor = c;

  palette.setColor(QPalette::Button, currentColor);
  button->setPalette(palette);
//  button->setPaletteBackgroundColor(currentColor);
  // and emit the information..
  emit colorChanged(waveIndex, r, g, b);
  cout << "new color for " << waveLength << "\t" << r << "\t" << g << "\t" << b << endl;
}

void ColorChooser::toggleColor(bool on){
    if(on){
	currentColor = constColor;
    }else{
	currentColor = QColor(0, 0, 0);
    }

    palette.setColor(QPalette::Button, currentColor);
    button->setPalette(palette);
    float r, g, b;
    color(&r, &g, &b);
    emit colorChanged(waveIndex, r, g, b);
}	 

void ColorChooser::color(float* r, float* g, float* b){
  int ri, gi, bi;
  currentColor.rgb(&ri, &gi, &bi);
  //  currentColor.getRgb(&ri, &gi, &bi);
  *r = (float)ri/255.0;
  *g = (float)gi/255.0;
  *b = (float)bi/255.0;
}

void ColorChooser::objectFind(){
  emit findObjects(waveLength);
}
