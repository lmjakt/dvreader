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

#include "channelWidget.h"
#include <qvalidator.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcolordialog.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <iostream>

using namespace std;

ChannelWidget::ChannelWidget(int ID, QString Ident, QColor c, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    // First make a couple of labels to identify the given thingy..
    id = ID;
    identifier = Ident;
    QString num;
    num.setNum(ID);
    QLabel* idLabel = new QLabel(num, this, "idLabel");
    QLabel* identLabel = new QLabel(Ident, this, "identLabel");
    
    // then the color box..
    currentColor = c; //QColor(255, 255, 255);  // default to white
    colorButton = new QPushButton(this, "button");
    colorButton->setFixedWidth(30);
    colorButton->setFixedHeight(30);
    colorButton->setPaletteBackgroundColor(currentColor);
    connect(colorButton, SIGNAL(clicked()), this, SLOT(changeColor()) );
    
    QLabel* fileLabel = new QLabel("File", this);
    fileCheckBox = new QCheckBox(this);

    // and then the scale box
    QLabel* scaleLabel = new QLabel("scale", this, "scaleLabel");
    scaleFactor = new QSpinBox(0, 1000, 1, this, "scaleFactor");
    scaleFactor->setValue(10);   // 10 is a magnification of 1.
    connect(scaleFactor, SIGNAL(valueChanged(int)), this, SLOT(sfChanged(int)) );

    // and then the parameters for finding spots..
    // Window Size
    QLabel* wsLabel = new QLabel("width", this, "wsLabel");
    windowSize = new QSpinBox(3, 31, 2, this, "windowSize");
    windowSize->setValue(7);
    
    // minPeakValue
    QLabel* mpvLabel = new QLabel("min pv", this, "mpvLabel");
    minPeakValue = new QLineEdit(this, "minPeakValue");
    // for which we need a validator.. (
    QDoubleValidator* pvValidator = new QDoubleValidator(minPeakValue);
    minPeakValue->setValidator(pvValidator);
    minPeakValue->setMinimumWidth(60);
    connect(minPeakValue, SIGNAL(returnPressed()), this, SLOT(pvChanged()) );

    // and the max thingy..
    QLabel* meLabel = new QLabel("max ev", this, "meLabel");
    maxEdgeValue = new QSpinBox(0, 100, 1, this, "maxEdgeValue");
    maxEdgeValue->setSuffix("%");
    connect(maxEdgeValue, SIGNAL(valueChanged(int)), this, SLOT(meChanged(int)) );

    // min Correlation.. although we may not use this ..
    QLabel* minCorLabel = new QLabel("min cor", this, "minCorLabel");
    minCorrelation = new QSpinBox(0, 100, 1, this, "minCorrelation");
    minCorrelation->setSuffix("%");
    // but this does not need to be connected to anything..

    QLabel* bgmLabel = new QLabel("BGM", this, "bgmLabel");
    backgroundMultiplier = new QSpinBox(1, 20, 1, this, "backgroundMultiplier");

    QLabel* clusterLabel = new QLabel("K clusterNo", this, "clusterLabel");
    clusterNumber = new QSpinBox(0, 40, 1, this, "clusterNumber");

    // and then finally a button to start the whole procedure...
    QPushButton* findButton = new QPushButton("Find", this, "findButton");
    connect(findButton, SIGNAL(clicked()), this, SLOT(findSpots()) );

    // and a button for finding spots in all frames..
    QPushButton* findAllButton = new QPushButton("Find (all)", this, "findAllButton");
    connect(findAllButton, SIGNAL(clicked()), this, SLOT(findAllSpots()));

    QPushButton* findAll3DButton = new QPushButton("All 3D", this);
    connect(findAll3DButton, SIGNAL(clicked()), this, SLOT(findAllSpots3D()) );

    // and then finally a layout for the whole thing. (am thinking primarily, just use a simple QHBox for now..
    QHBoxLayout* hbox = new QHBoxLayout(this, 1, 1, "hbox");
    hbox->addWidget(colorButton);
    hbox->addSpacing(5);
    hbox->addWidget(idLabel);
    hbox->addSpacing(20);
    hbox->addWidget(identLabel);
    hbox->addSpacing(20);
    hbox->addWidget(fileLabel);
    hbox->addWidget(fileCheckBox);
    hbox->addSpacing(4);
    hbox->addWidget(scaleLabel);
    hbox->addWidget(scaleFactor);
    hbox->addStretch();
    hbox->addWidget(wsLabel);
    hbox->addWidget(windowSize);
    hbox->addWidget(mpvLabel);
    hbox->addWidget(minPeakValue);
    hbox->addWidget(meLabel);
    hbox->addWidget(maxEdgeValue);
    hbox->addWidget(minCorLabel);
    hbox->addWidget(minCorrelation);
    hbox->addWidget(bgmLabel);
    hbox->addWidget(backgroundMultiplier);
    hbox->addWidget(clusterLabel);
    hbox->addWidget(clusterNumber);
    hbox->addWidget(findButton);
    hbox->addWidget(findAllButton);
    hbox->addWidget(findAll3DButton);
}

void ChannelWidget::currentState(QString& ident, int& Id, int& ws, float& mpv, int& mev, int& minCorr, int& r, int& g, int& b, int& k, int& bgm, bool& expFile){
    ident = identifier;
    Id = id;
    ws = windowSize->value();
    mpv = minPeakValue->text().toFloat();
    mev =  maxEdgeValue->value();
    minCorr = minCorrelation->value();
    r = g = b = 255;  // ?
    k = clusterNumber->value();
    bgm = backgroundMultiplier->value();
    expFile = fileCheckBox->isChecked();
}

bool ChannelWidget::setState(QString ident, int Id, int ws, float mpv, int mev, int minCorr, int r, int g, int b, int k, int bgm, bool expFile){
    if(ident != identifier){
	cerr << "ChannelWidget attempted to set state to a widget with identifier " << identifier.latin1() << "  with non-matching identifier: " << ident.latin1() << endl;
	return(false);
    }
    if(Id != id){
	cerr << "ChannelWidget set state. Warning, supplied id : " << Id << " does not match own id: " << id << "  but ignoring for now." << endl;
    }
    windowSize->setValue(ws);
    QString mpvString;
    mpvString.setNum(mpv);
    minPeakValue->setText(mpvString);
    maxEdgeValue->setValue(mev);
    minCorrelation->setValue(minCorr);
    clusterNumber->setValue(k);
    backgroundMultiplier->setValue(bgm);
    fileCheckBox->setChecked(expFile);
}

void ChannelWidget::findSpots(){
    // leave blank for now..
    mpValue = minPeakValue->text().toFloat();
    int me = maxEdgeValue->value();
    float mev = float(me) / float(100);
//    float mev = mpValue * float(me) / float(100);
    int wsize = windowSize->value();
    emit findspots(id, wsize, mpValue, mev, 0.0, 255, 255, 255);
}

void ChannelWidget::findAllSpots(){
    mpValue = minPeakValue->text().toFloat();
    int me = maxEdgeValue->value();
    float mev = float(me) / float(100);
    int wsize = windowSize->value();
    float bgm = float(backgroundMultiplier->value());
    emit findallspots(id, wsize, mpValue, mev, 0.0, 255, 255, 255, clusterNumber->value(), bgm, fileCheckBox->isChecked());
}    

void ChannelWidget::findAllSpots3D(){
    mpValue = minPeakValue->text().toFloat();
    int me = maxEdgeValue->value();
    float mev = float(me) / float(100);
    int wsize = windowSize->value();
    float bgm = float(backgroundMultiplier->value());
    emit findallspots3D(id, wsize, mpValue, mev, 0.0, 255, 255, 255, clusterNumber->value(), bgm, fileCheckBox->isChecked());
}   

void ChannelWidget::pvChanged(){
    mpValue = minPeakValue->text().toFloat();
    emit newPeakValue(id, mpValue);
}

void ChannelWidget::meChanged(int me){
    float mpv = minPeakValue->text().toFloat();
    if(mpValue != mpv){
	mpValue = mpv;
	emit newPeakValue(id, mpValue);
    }

    float v = mpValue * float(me) / float(100);
    emit newMaxEdgeValue(id, v);
}

void ChannelWidget::sfChanged(int sf){
    emit newScaleFactor(id, float(sf/10.0));
}

void ChannelWidget::changeColor(){
    currentColor = QColorDialog::getColor(currentColor);
    colorButton->setPaletteBackgroundColor(currentColor);
}

     
    
    
