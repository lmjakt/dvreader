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

#include "parameterChooser.h"
#include <qpushbutton.h>
//Added by qt3to4:
#include <QHBoxLayout>

ParameterChooser::ParameterChooser(parameterData parData, QString label, int windex, int wlength, QColor c, QWidget* parent, const char* name)
    : ColorChooser(label, windex, wlength, c, parent, name)
{

    // make the color chooser and a button to display it.. 
    chooser = new DistChooser(label.latin1(), 100);
    cout << "ParameterChooser just finished making a distchooser" << endl;
    // chooser only takes a vector for setting values.. so we'll make a vector
    // from the parData. this is a bit slow, but..
    data = parData;
    vector<float> v(data.width * data.height);
    for(uint i=0; i < (data.width * data.height); ++i){
	v[i] = data.values[i];
    }
    cout << "v is " << v.size() << " min : " << parData.minValue << " max : " << parData.maxValue 
	 << "  : " << parData.width << "," << parData.height << endl;
    chooser->setData(v, data.minValue, data.maxValue);
    connect(chooser, SIGNAL(newRanges(float, float)), this, SLOT(setBiasAndScale(float, float)) );
    connect(chooser, SIGNAL(newRanges(float, float)), this, SIGNAL(newRanges(float, float)) );
    chooser->resize(300, 200);
    cout << "data set for chooser and size set to 300,200" << endl;
    bias = 0;
    scale = 1.0 / (data.maxValue - data.minValue);   // hence we can just multiply by this number.. 

    // make a button..
    QPushButton* dispButton = new QPushButton("dist", this, "dispButton");
    connect(dispButton, SIGNAL(clicked()), this, SLOT(displayDistChooser()) );
    cout << "made a dispButton" << endl;
    QHBoxLayout* hbox = new QHBoxLayout();
    cout << "made a layout box" << endl;
    vbox->addLayout(hbox);
    hbox->addStretch();
    hbox->addWidget(dispButton);
    cout << "Parameter chooser construction finished" << endl;
}

ParameterChooser::~ParameterChooser(){
    delete chooser;
}

void ParameterChooser::displayDistChooser(){
    chooser->show();
    chooser->raise();
}

void ParameterChooser::setBiasAndScale(float lowT, float highT){
    // bias is equal to minValue - the thingy..
    bias = data.minValue - lowT;   // which is always a negative number.. 
    scale = 1.0 / (highT - lowT);
}
