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

#include "slider.h"
#include <qfont.h>
#include <iostream>

using namespace std;

Slider::Slider(QRect r, int Min, int Max){
    min = low = Min;
    max = high = Max;
    outerBox = r;
    innerBox = r;   // default to a full range..
    innerColor = QColor(136, 164, 201);
    outerColor = QColor(255, 255, 255);
}

Slider::~Slider(){
    // do nothing..
}

void Slider::setInnerColor(QColor c){
    innerColor = c;
}

void Slider::setOuterColor(QColor c){
    outerColor = c;
}

void Slider::draw(QPainter* p){
    // first draw the slider outline for the box..
    int bwidth = 2;   // draw a box on either side of the thingy..
    p->setPen(QPen(outerColor, 1));
    p->setBrush(outerColor);
    int lh = outerBox.height() % 2 ? 2 : 1;    // the width is 2 if even and one if not..
    p->drawRect(outerBox.left(), outerBox.bottom() - outerBox.height()/2, outerBox.width(), lh);  // 
    p->drawRect(outerBox.left()-bwidth, outerBox.top(), bwidth, outerBox.height());
    p->drawRect(outerBox.right(), outerBox.top(), bwidth, outerBox.height());
    
    p->setPen(Qt::NoPen);
    p->setBrush(innerColor);
    p->drawRect(innerBox);
    QFont currentFont = p->font();
    p->setFont(QFont("Arial", currentFont.pointSize()-1));
    p->setPen(QPen(outerColor, 1));
    QString num;
    num.setNum(low);
    int twidth = 100;
    int theight = 200;
    p->drawText(innerBox.left(), innerBox.top() - theight - 2, twidth, theight, Qt::AlignLeft|Qt::AlignBottom, num);
    num.setNum(high);
    p->drawText(innerBox.right() - twidth, innerBox.bottom() + 2, twidth, theight, Qt::AlignRight|Qt::AlignTop, num);
    // and thats all for now... 
}

void Slider::setLimits(int Low, int High){
    //cout << "setLimits low : " << low << " --> " << Low << "\thigh : " << high << " --> " << High << endl;
    if(Low >= High){
	return;
    }
    if(Low < min){ Low = min; }
    if(High > max){ High = max; }
    low = Low;
    high = High;
    // and adjust the inner box appropriately..
    int l = outerBox.left() + (outerBox.width() * (low - min))/(max - min);
    int r = outerBox.right() - (outerBox.width() * (max - high))/(max - min);
    innerBox.setLeft(l);
    innerBox.setRight(r);
    // and that should be it..
}

void Slider::setPixelLimits(int Low, int High){
    if(Low < High && Low >= outerBox.left() && High <= outerBox.right()){
	low = min + ((Low - outerBox.left()) * (max - min) )/outerBox.width();
	high = min + ((High - outerBox.left()) * (max - min) )/outerBox.width();
	innerBox.setLeft(Low);
	innerBox.setRight(High);
    }
}

// the above function is ok, but it's probably easier to use one that adjusts..
void Slider::adjustLimits(int dl, int dh){           // dl and dh are delta low and delta high respectively..
    // first check that the resulting values will make some sense..
    //cout << "adjustLimits dl : " << dl << "\tdh : " << dh << endl;
     if(innerBox.left() + dl >= innerBox.right() + dh){
 	return;
     }
//     if(innerBox.left() + dl < outerBox.left()){
// 	return;
//     }
//     if(innerBox.right() + dh > outerBox.right()){
// 	return;
//     }
    
    //cout << "dh and dh ok " << endl;
    // and then we can do basically the same as above..
    // cout << "Before low is now : " << low << "  and high : " << high << endl;
    
    //innerBox.setLeft(innerBox.left() + dl);
    //innerBox.setRight(innerBox.right() + dh);
    low = low + ((dl * (max - min)) / outerBox.width());
    high = high + ((dh * (max - min)) / outerBox.width());
//    low = min + ((innerBox.left() - outerBox.left()) * (max - min) )/outerBox.width();
//    high = min + ((innerBox.right() - outerBox.left()) * (max - min) )/outerBox.width();
    // cout << "After  low is now : " << low << "  and high : " << high << endl;
}

void Slider::setPosition(QRect r){
    outerBox = r;
    innerBox = r;
    // and use the setLimits thingy to set the position of the box..
    setLimits(low, high);
}

// that would seem to be all the functions unless I want to do someth
void Slider::setRange(int Min, int Max){
    min = Min;
    max = Max;
    cout << "setRange min " << min << "  max " << max << "  low " << low << " high " << high << endl;
    if(low <= min){ low = min; }
    if(high > max || high <= low){ high = max; }
    cout << "                           low " << low << "  high " << high << endl << endl;
    setLimits(low, high);  // not necessary as it gets done in the redraw anyway.. 
}

int Slider::begin(){
    return(low);
}

int Slider::end(){
    return(high);
}

bool Slider::innerContains(int x, int y){
    return(innerBox.contains(x, y));
}

bool Slider::outerContains(int x, int y){
    return(outerBox.contains(x, y));
}
