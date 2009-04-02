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

#include "spotsWidget.h"
#include <iostream>
#include <qlayout.h>
#include <qcolordialog.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QHBoxLayout>

using namespace std;

SpotsWidget::SpotsWidget(QString waveLabel, int WaveLength, int WaveIndex, int r, float mpv, float mev, threeDPeaks* Peaks, QWidget* parent, const char* name)
    : QWidget(parent, name)
{

    drops = Peaks;
    labelString = waveLabel;
    waveLength = WaveLength;
    waveIndex = WaveIndex;   // necessary in order to know which stuff to use.. 
    radius = r;
    minPeakValue = mpv;
    maxEdgeValue = mev;

    // set up the current color..
    palette = QPalette();
    currentColor = QColor(int(drops->r * 255.0), int(drops->g * 255.0), int(drops->b * 255.0));
    
    QLabel* label = new QLabel(labelString, this, "label");
    colorButton = new QPushButton(this, "colorButton");
    palette.setColor(QPalette::Button, currentColor);
    colorButton->setPalette(palette);
    colorButton->setFixedWidth(30);
    colorButton->setFixedHeight(30);
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setColor()) );

    // set reptype to no type..
    currentType = NO_REP;
    int bsize = 20;

    // then make the set of dropButtons..
    DropButton* dbutton = new DropButton(DOT, bsize, this, "dbutton");
    dropButtons.insert(make_pair(DOT, dbutton));
    connect(dbutton, SIGNAL(toggled(bool)), this, SLOT(repButtonToggled(bool)) );

    DropButton* cbutton = new DropButton(CIRCLE, bsize, this, "cbutton");
    dropButtons.insert(make_pair(CIRCLE, cbutton));
    connect(cbutton, SIGNAL(toggled(bool)), this, SLOT(repButtonToggled(bool)) );

    DropButton* fpbutton = new DropButton(FIVE_POINTS, bsize, this, "fpbutton");
    dropButtons.insert(make_pair(FIVE_POINTS, fpbutton));
    connect(fpbutton, SIGNAL(toggled(bool)), this, SLOT(repButtonToggled(bool)) );

    DropButton* sbutton = new DropButton(SQUARE, bsize, this, "sbutton");
    dropButtons.insert(make_pair(SQUARE, sbutton));
    connect(sbutton, SIGNAL(toggled(bool)), this, SLOT(repButtonToggled(bool)) );

    DropButton* bvbutton = new DropButton(BY_VALUE, bsize, this, "bvbutton");
    dropButtons.insert(make_pair(BY_VALUE, bvbutton));
    connect(bvbutton, SIGNAL(toggled(bool)), this, SLOT(repButtonToggled(bool)) );

    QHBoxLayout* box = new QHBoxLayout(this);
    box->addWidget(label);
    box->addStretch();
    box->addWidget(dbutton);
    box->addWidget(cbutton);
    box->addWidget(fpbutton);
    box->addWidget(sbutton);
    box->addWidget(bvbutton);
    box->addSpacing(5);
    box->addWidget(colorButton);
}

SpotsWidget::~SpotsWidget(){
    delete drops;
}

void SpotsWidget::setColor(){
    QColor c = QColorDialog::getColor(currentColor);
    int ri, gi, bi;
//    float r, g, b;
    
    c.rgb(&ri, &gi, &bi);
    //  c.getRgb(&ri, &gi, &bi);
    drops->r = (float)ri/255.0;
    drops->g = (float)gi/255.0;
    drops->b = (float)bi/255.0;
    currentColor = c;
    
    palette.setColor(QPalette::Button, currentColor);
    colorButton->setPalette(palette);
//    colorButton->setPaletteBackgroundColor(currentColor);
    // and emit the information..
    emit colorChanged(waveLength, drops->r, drops->g, drops->b);
    emit colorChanged();
    cout << "new color for " << waveLength << "\t" << drops->r << "\t" << drops->g << "\t" << drops->b << endl;
}

void SpotsWidget::color(float& r, float& g, float& b){
    r = drops->r;
    g = drops->g;
    b = drops->b;
}

void SpotsWidget::repButtonToggled(bool on){
    
    if(!on){
	currentType = NO_REP;
	emit repTypeChanged();
//	emit repTypeChanged(NO_REP);
	return;
    }
    // otherwise find out which button..
    DropButton* button = (DropButton*)sender();
    if(!button){
	cerr << "SpotsWidget repButtonToggled, but no button owns up to being toggled" << endl;
	return;
    }
    currentType = button->repType();
    // and now we have to make sure that all the othe buttons are set off..
    map<DropRepresentation, DropButton*>::iterator it;
    for(it = dropButtons.begin(); it != dropButtons.end(); it++){
//	cout << "Checking buttons currentType = " << currentType << "\tbutton Type : " << (*it).first << "  button reports : " << (*it).second->repType() <<  endl;
	if((*it).first != currentType){
	    (*it).second->blockSignals(true);
	    (*it).second->switchOff();
	    (*it).second->blockSignals(false);
	}
    }
//    emit repTypeChanged(currentType);
    emit repTypeChanged();
}
