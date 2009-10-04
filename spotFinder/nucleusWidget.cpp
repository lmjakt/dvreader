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

#include "nucleusWidget.h"
#include <iostream>
#include <qvalidator.h>
#include <qpushbutton.h>
#include <qlabel.h>
//#include <qlayout.h>
#include <qsizepolicy.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStringList>

using namespace std;

NucleusWidget::NucleusWidget(QString buttonLabel, QWidget* parent, const char* name)
    : QWidget(parent, name)
{

  QLabel* channelLabel = new QLabel("channel", this);
    channels = new QButtonGroup(this);
    //    channels = new Q3ButtonGroup(1, Qt::Vertical, "channel", this);
//    channels = new Q3ButtonGroup(1, Q3GroupBox::Vertical, "channel", this);
    //  channels->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

    QLabel* minLineLabel = new QLabel("Minimum Intensity", this, "minLineLabel");
    minLine = new QLineEdit(this, "minLine");
    
    //QDoubleValidator* minValidator = new QDoubleValidator(minLine);
    //minLine->setValidator(minValidator);
    
    QPushButton* findButton = new QPushButton(buttonLabel, this, "findButton");
//    QPushButton* findButton = new QPushButton("Find Nuclei", this, "findButton");
    connect(findButton, SIGNAL(clicked()), this, SLOT(findNuclei()) );
    
    //vbox = new QVBoxLayout(this);
    //vbox->setSpacing(1);
    //vbox->setMargin(1);
    QHBoxLayout* mainBox = new QHBoxLayout(this);
    mainBox->setSpacing(1);
    hbox = new QHBoxLayout();
    mainBox->addLayout(hbox);
    hbox->addWidget(channelLabel);
    //    vbox->addLayout(hbox);
//    QHBoxLayout* hbox = new QHBoxLayout(this, 1, 1, "hbox");
//    mainBox->addWidget(channels);
    mainBox->addWidget(minLineLabel);
    mainBox->addWidget(minLine);
    mainBox->addStretch();
    mainBox->addWidget(findButton);
}

void NucleusWidget::setChannels(vector<QString> Channels){
    /// first remove any buttons that we've already got..
    for(uint i=0; i < channelButtons.size(); i++){
	channels->remove(channelButtons[i]);
	cout << "deleting channel " << i << endl;
	delete channelButtons[i];
    }
    channelButtons.resize(0);
    // and then let's make some new buttons..
    for(uint i=0; i < Channels.size(); i++){
      QRadioButton* button = new QRadioButton(Channels[i]);
      channels->addButton(button, i);
	hbox->addWidget(button);
	channelButtons.push_back(button);
	button->show();
    }
}
    
void NucleusWidget::findNuclei(){
    bool ok;
    int selectedId = channels->checkedId();
    if(selectedId == -1){
	cerr << "NucleusWidget : no wavelengt selected" << endl;
	return;
    }
    // first let's see if we can get the following set of values,,
    // minValue, xw, yw, zw, percentile
    // where these refer to the cell size used for calculating background estimates
    // 
    QStringList lst = minLine->text().split(",");
    if(lst.size() == 5){
	bool ok1, ok2, ok3, ok4, ok5;
	float mv = lst[0].toFloat(&ok1);
	int xw = lst[1].toInt(&ok2);
	int yw = lst[2].toInt(&ok3);
	int zw = lst[3].toInt(&ok4);
	float pcnt = lst[4].toInt(&ok5);
	if(ok1 && ok2 & ok3 && ok4 && ok5){
	    emit findStuff(selectedId, mv, xw, yw, zw, pcnt);
	    return;
	}
	cout << "did not find what I was looking for from the line " << minLine->text().ascii() << endl;
	for(uint i=0; i < lst.size(); ++i){
	    cout << i << " : " << lst[i].ascii() << endl;
	}
    }

    float minValue = minLine->text().toFloat(&ok);
    if(!ok){
	cerr << "NucleusWidget unable to parse float from minLine" << endl;
	return;
    }

    emit findNuclearPerimeters(selectedId, minValue);
}


