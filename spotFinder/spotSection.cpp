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

#include <qcolor.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "spotSection.h"
#include <iostream>
#include <qlabel.h>
#include <qlayout.h>

using namespace std;

SpotSection::SpotSection(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    x_y_view = new SpotSectionView(this, "x_y_view");
    x_z_view = new SpotSectionView(this, "x_z_view");
    
    XY_section = new QSpinBox(0, 0, 1, this, "XY_section");
    XZ_section = new QSpinBox(0, 0, 1, this, "XZ_section");
 
    connect(XY_section, SIGNAL(valueChanged(int)), this, SLOT(setXYsection(int)) );
    connect(XZ_section, SIGNAL(valueChanged(int)), this, SLOT(setXZsection(int)) );
   
    QLabel* XY_label = new QLabel("XY section", this, "XY_label");
    QLabel* XZ_label = new QLabel("XZ section", this, "XZ_label");

    QVBoxLayout* vbox = new QVBoxLayout(this, 1, 1);
    QHBoxLayout* viewBox = new QHBoxLayout();
    vbox->addLayout(viewBox);
    QHBoxLayout* controlBox = new QHBoxLayout();
    vbox->addLayout(controlBox);
    
    viewBox->addWidget(x_y_view);
    viewBox->addWidget(x_z_view);
    controlBox->addWidget(XY_label);
    controlBox->addWidget(XY_section);
    controlBox->addWidget(XZ_label);
    controlBox->addWidget(XZ_section);
}


// all members have spot section as a parent, so may not need to be deleted.
// If I implement the destructor, I may need to destroy everything.
// SpotSection::~SpotSection(){
// }

void SpotSection::setDrop(simple_drop& drop){
    currentDrop = drop;
    // all drops are cubes.. with sides of length 2 * radius + 1;
    XY_section->setRange(0, currentDrop.radius * 2);
    XZ_section->setRange(0, currentDrop.radius * 2);

    XY_section->setValue(currentDrop.radius);
    XZ_section->setValue(currentDrop.radius); // this will set off the signal, which we then catch in the thingy. (but doesn't seem to do anything)

    setXYsection(currentDrop.radius);
    setXZsection(currentDrop.radius);
}

void SpotSection::setXYsection(int sec){
    // first make sure that sec is good..
    if(sec < 0 || sec > currentDrop.radius * 2){
	cerr << "setXYsection, sec is a bad value : " << sec << endl;
	return;
    }
    // we need to make a vector of QColors..
    // for now just use 0-1 as 0->255

    int s = currentDrop.radius * 2 + 1;
    vector<QColor> colors(s * s);   // but how do we interpret colours... ?
    
    for(int y=0; y < s; y++){
	for(int x = 0; x < s; x++){
	    int gi = sec * s * s + y * s + x;
	    int i = y * s + x;
	    int r = int(255.0 * currentDrop.values[gi]);
	    r = r > 255 ? 255 : r;
	    int b = 0;
//	    int b = currentDrop.kernelValues[gi] > 0 ? 125 : 0;
	    int g = 0;   // which we'll leave at that.. 
	    colors[i].setRgb(r, g, b);    // but that might not be the best approach.. 
	}
    }
    x_y_view->setView(colors, s, s);
}

void SpotSection::setXZsection(int sec){
    if(sec < 0 || sec > currentDrop.radius * 2){
	cerr << "setXZsection, sec is a bad value : " << sec << endl;
	return;
    }
    int s = currentDrop.radius * 2 + 1;
    vector<QColor> colors(s * s);
    for(int z=0; z < s; z++){
	for(int x=0; x < s; x++){
	    int gi = z * s * s + sec * s + x;
	    int i = z * s + x;
	    int r = int(255.0 * currentDrop.values[gi]);
	    int b = 0;
//	    int b = currentDrop.kernelValues[gi] > 0 ? 125 : 0;
	    r = r > 255 ? 255 : r;
	    int g = 0;
	    colors[i].setRgb(r, g, b);
	}
    }
    x_z_view->setView(colors, s, s);
}

