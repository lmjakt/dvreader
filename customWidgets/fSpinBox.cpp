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

#include "fSpinBox.h"
#include <qvalidator.h>
#include <qstring.h>

FSpinBox::FSpinBox(float min, float max, int steps, QWidget* parent, const char* name)
  : QSpinBox(0, steps, 1, parent, name)
{
  if(min < max){
    maxValue = max;
    minValue = min;
  }
  if(min > max){
    maxValue = min;
    minValue = max;
  }
  if(min == max){
    minValue = min;
    maxValue = max +1;   // avoid divide by 0 errors.. 
  }
  stepNo = steps;

//  setValidator(new QDoubleValidator(minValue, maxValue, 6, this));  // -----------
  // and that's really all that I need to do
}

QString FSpinBox::mapValueToText(int value){
  float f = minValue + ((float)value * (maxValue-minValue) /(float)stepNo);
  QString num;
  num.setNum(f, 'g', 4);
  return(num);
}

int FSpinBox::mapTextToValue(bool* ok){
  return( (int) ((text().toFloat(ok) - minValue)*((float)stepNo)/(maxValue-minValue)));
}

float FSpinBox::fvalue(){
  float f = minValue + ((float)value() * (maxValue-minValue) /(float)stepNo);
  return(f);
}

void FSpinBox::setFValue(float f){
  //cout << "SPin box trying to set value " << endl;
  int v = (int)( (f - minValue)*(float)stepNo/(maxValue-minValue) );
  //cout << "maxValue is : " << maxValue << "   minValue is " << minValue << endl
  //     << "stepNo is " << stepNo << endl;
  //cout << " and v is now " << v << endl;
  setValue(v);
}


