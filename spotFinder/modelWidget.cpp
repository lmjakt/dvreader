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

#include "modelWidget.h"
#include <qlabel.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QHBoxLayout>

using namespace std;

ModelWidget::ModelWidget(int Width, int Height, int Depth, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    w = Width;
    h = Height;
    d = Depth;

    xBeginBox = new QSpinBox(0, w, 1, this, "xBeginBox");
    QLabel* xLabel = new QLabel("X", this, "xLabel");
    widthBox = new QSpinBox(20, w, 1, this, "widthBox");
    QLabel* widthLabel = new QLabel("Width", this, "widthLabel");
    yBeginBox = new QSpinBox(0, h, 1, this, "yBeginBox");
    QLabel* yLabel = new QLabel("Y", this, "yLabel");
    heightBox = new QSpinBox(20, h, 1, this, "heightBox");
    QLabel* heightLabel = new QLabel("Height", this, "heightLabel");
    zBeginBox = new QSpinBox(0, d, 1, this, "zBeginBox");
    QLabel* zLabel = new QLabel("Z", this, "zLabel");
    depthBox = new QSpinBox(5, d, 1, this, "depthBox");
    QLabel* depthLabel = new QLabel("Depth", this, "depthLabel");

    // and then a button..
    QPushButton* doButton = new QPushButton("Make Model", this, "doButton");
    connect(doButton, SIGNAL(clicked()), this, SLOT(requestModel()) );
    
    // and a button to request calculation of new volumes (we could set this to be with a given radius as 
    // as well..
    QPushButton* volButton = new QPushButton("Update Volumes", this, "volButton");
    connect(volButton, SIGNAL(clicked()), this, SIGNAL(recalculateSpotVolumes()) );

    // and a layout..
    QHBoxLayout* box = new QHBoxLayout(this, 0, 2);
    channelBox = new QHBoxLayout();
    box->addStretch();
    box->addLayout(channelBox);
    box->addWidget(xLabel);
    box->addWidget(xBeginBox);
    box->addWidget(widthLabel);
    box->addWidget(widthBox);
    box->addWidget(yLabel);
    box->addWidget(yBeginBox);
    box->addWidget(heightLabel);
    box->addWidget(heightBox);
    box->addWidget(zLabel);
    box->addWidget(zBeginBox);
    box->addWidget(depthLabel);
    box->addWidget(depthBox);
    box->addWidget(doButton);
    box->addWidget(volButton);
}

void ModelWidget::setChannels(vector<QString> Channels){
    // first remove old checkboxes..
    for(uint i=0; i < channelBoxes.size(); i++){
	delete channelBoxes[i];
    }
    channelBoxes.resize(0);
    for(uint i=0; i < Channels.size(); i++){
	channelBoxes.push_back(new QCheckBox(Channels[i], this));
	channelBox->addWidget(channelBoxes.back());
	channelBoxes.back()->show();
    }
}

void ModelWidget::requestModel(){
    set<int> waves;
    for(uint i=0; i < channelBoxes.size(); i++){
	if(channelBoxes[i]->isChecked()){
	    waves.insert(i);
	}
    }
    emit makeModel(xBeginBox->value(), widthBox->value(), yBeginBox->value(), heightBox->value(), zBeginBox->value(), depthBox->value(), waves);
}
