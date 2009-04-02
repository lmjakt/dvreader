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

#include "blurWidget.h"
#include <QPushButton>
#include <QLabel>
#include <iostream>

BlurWidget::BlurWidget(QWidget* parent)
    : QWidget(parent)
{

    radiusBox = new QSpinBox(this);
    sigmaBox = new QDoubleSpinBox(this);
    orderBox = new QDoubleSpinBox(this);
    
    radiusBox->setRange(1, 100);
    sigmaBox->setRange(1.0, 100);
    orderBox->setRange(1.1, 10);

    sigmaBox->setSingleStep(0.1);
    orderBox->setSingleStep(0.1);

    radiusBox->setValue(10);
    sigmaBox->setValue(20.0);
    orderBox->setValue(2.0);

    QLabel* r_label = new QLabel("radius", this);
    QLabel* s_label = new QLabel("sigma", this);
    QLabel* o_label = new QLabel("order", this);

    // and a QPushButton..
    QPushButton* b_button = new QPushButton("Blur", this);
    connect(b_button, SIGNAL(clicked()), this, SLOT(blur()) );
    // we then need to stick this into some kind of a layout..
    QHBoxLayout* mainBox = new QHBoxLayout(this);
    mainBox->setSpacing(1);
    hbox = new QHBoxLayout();
    mainBox->addLayout(hbox);
    mainBox->addStretch();
    mainBox->addWidget(r_label);
    mainBox->addWidget(radiusBox);
    mainBox->addWidget(s_label);
    mainBox->addWidget(sigmaBox);
    mainBox->addWidget(o_label);
    mainBox->addWidget(orderBox);
    mainBox->addWidget(b_button);
    
}

void BlurWidget::setChannels(std::vector<QString> channels){
    for(uint i=0; i < channelBoxes.size(); i++){
	std::cout << "deleting channel " << i << std::endl;
	channelBoxes[i]->hide();
	delete channelBoxes[i];
    }
    channelBoxes.resize(0);
    // and then let's make some new buttons..
    for(uint i=0; i < channels.size(); i++){
	QCheckBox* box = new QCheckBox(channels[i], this);
	channelBoxes.push_back(box);
	hbox->addWidget(box);
	box->show();
    }
}

void BlurWidget::blur(){
    // first get the channels that we are interested in..
    std::set<uint> selected;  // the selected channels..
    for(uint i=0; i < channelBoxes.size(); ++i){
	if(channelBoxes[i]->isChecked()){
	    selected.insert(i);
	}
    }
    // then make sure that someting was selected..
    if(!selected.size()){
	std::cerr << "BlurWidget::blur() No channel selected" << std::endl;
	return;
    }
    // then try to get the appropriate values..
    int r = radiusBox->value();
    double sigma = sigmaBox->value();
    double order = sigmaBox->value();
    // and then just emit ..
    emit blur(selected, r, sigma, order);  // and hope that someone catches the signal..
}

