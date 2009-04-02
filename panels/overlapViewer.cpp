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

#include "overlapViewer.h"
#include <qlayout.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <string.h>
#include <iostream>

using namespace std;

OverlapViewer::OverlapViewer(unsigned int textureSize, QWidget* parent, const char* name)
    : QWidget(parent, name)
{

    // check that texturesize is ok..
    texSize = 1;
    while(texSize < textureSize){
	texSize *= 2;
    }
    if(texSize != textureSize){
	cerr << "OverlapViewer::OverlapViewer inappropriate textureSize specified (" << textureSize
	     << ") texture size set to " << texSize << endl;
    }
    
    // create the glImage..
    glImage = new GLImage(1, 1, texSize, 1.0, this, "glImage");
    mergedImage = imageA = imageB = 0;

    float* tempBuffer = new float[texSize * texSize * 3];
    memset((void*)tempBuffer, 0, texSize * texSize * 3 * sizeof(float));
//    glImage->setImage(tempBuffer, texSize, texSize, 0, 0);
    delete tempBuffer;
  
    // the spinboxes and stuff..
    dxBox = new QSpinBox(this, "dxBox");
    dyBox = new QSpinBox(this, "dyBox");
    scaleBox = new QSpinBox(0, 1000, 1, this, "scaleBox");
    biasBox = new QSpinBox(-100, 100, 1, this, "biasBox");

    // connect these to the appropriate slots..
    connect(dxBox, SIGNAL(valueChanged(int)), this, SLOT(set_dx(int)) );
    connect(dyBox, SIGNAL(valueChanged(int)), this, SLOT(set_dy(int)) );
    connect(scaleBox, SIGNAL(valueChanged(int)), this, SLOT(setScale(int)) );
    connect(biasBox, SIGNAL(valueChanged(int)), this, SLOT(setBias(int)) );

    // a couple of checkboxes to determine which bit we'll be using...
    QCheckBox* aCheck = new QCheckBox("A", this, "aCheck");
    QCheckBox* bCheck = new QCheckBox("B", this, "bCheck");

    aCheck->setChecked(true);
    bCheck->setChecked(true);
    drawA = true;
    drawB = true;
    
    connect(aCheck, SIGNAL(toggled(bool)), this, SLOT(paintA(bool)) );
    connect(bCheck, SIGNAL(toggled(bool)), this, SLOT(paintB(bool)) );

    // some labels..
    // first ones that we'll set ..
    scaleValueLabel = new QLabel("1.0", this, "scaleValueLabel");
    biasValueLabel = new QLabel("0.0", this, "biasValueLabel");
    
    QLabel* dxLabel = new QLabel("dx", this, "dxLabel");
    QLabel* dyLabel = new QLabel("dy", this, "dyLabel");
    QLabel* scaleLabel = new QLabel("scale", this, "scaleLabel");
    QLabel* biasLabel = new QLabel("bias", this, "biasLabel");

    // and then a layout
    QVBoxLayout* mainBox = new QVBoxLayout(this);
    QGridLayout* controlGrid = new QGridLayout(2, 8);
    mainBox->addWidget(glImage);
    mainBox->addLayout(controlGrid);
    mainBox->setStretchFactor(mainBox, 1);
    mainBox->setStretchFactor(controlGrid, 0);
    
    controlGrid->addWidget(dxBox, 0, 0);
    controlGrid->addWidget(dxLabel, 1, 0);
    controlGrid->addWidget(dyBox, 0, 1);
    controlGrid->addWidget(dyLabel, 1, 1);
    controlGrid->setColStretch(2, 1);
    controlGrid->addWidget(biasLabel, 0, 3);
    controlGrid->addWidget(biasBox, 0, 4);
    controlGrid->addWidget(biasValueLabel, 1, 4);
    controlGrid->addWidget(scaleLabel, 0, 5);
    controlGrid->addWidget(scaleBox, 0, 6);
    controlGrid->addWidget(scaleValueLabel, 1, 6);
    controlGrid->addWidget(aCheck, 0, 7);
    controlGrid->addWidget(bCheck, 1, 7);

}

OverlapViewer::~OverlapViewer(){
    // maybe delete some buffers,, but.. 
//     if(imageA){
// 	delete imageA;
//     }
//     if(imageB){
// 	delete imageB;
//     }
    if(mergedImage){
	delete mergedImage;
    }
}

void OverlapViewer::setImage(float* a, float* b, unsigned int Width, unsigned int Height, int delta_x, int delta_y){
//     if(imageA){
// 	delete imageA;
//     }
//     if(imageB){
// 	delete imageB;
//     }
    imageA = a;
    imageB = b;
    imageWidth = Width;
    imageHeight = Height;
    dx = delta_x;
    dy = delta_y;

    dxBox->blockSignals(true);
    dxBox->setRange(-imageWidth, +imageWidth);
    dxBox->setValue(dx);
    dxBox->blockSignals(false);

    dyBox->blockSignals(true);
    dyBox->setRange(-imageHeight, +imageHeight);
    dyBox->setValue(dy);
    dyBox->blockSignals(false);
    
    // and then set up a buffer sufficiently large to hold the merged image with no overlap.. 
    if(mergedImage){
	delete mergedImage;
	mergedImage = 0;
    }
    mergedWidth = (imageWidth * 3) > texSize ? texSize : (imageWidth * 3);
    mergedHeight = (imageHeight * 3) > texSize ? texSize : (imageHeight * 3);

    // we then have to consider how to fit the merged image into this box.. 
    mergedImage = new float[mergedWidth * mergedHeight * 3];    // it's an rgb triplet after all.. 

    // We now have to get the maximum value for each one of these.
    // Use the max value to set scale such that maxValue --> 1.0
    // 
    
    float maxValueA = findMax(imageA, imageWidth * imageHeight);
    float maxValueB = findMax(imageB, imageWidth * imageHeight);
    float maxValue = maxValueA > maxValueB ? maxValueA : maxValueB;
    
    defaultScale = 1.0 / maxValue;
    scaleFactor = defaultScale;
    scaleBox->blockSignals(true);
    scaleBox->setValue(100);   // represents 100% of the default scale..
    scaleBox->blockSignals(false);
    
    biasFactor = 0;
    biasBox->blockSignals(true);
    biasBox->setValue(0);
    biasBox->blockSignals(false);
    
    // and then simply .. 
    setImage();
}

void OverlapViewer::set_dx(int delta_x){
    dx = delta_x;
    setImage();
}

void OverlapViewer::set_dy(int delta_y){
    dy = delta_y;
    setImage();
}

void OverlapViewer::setScale(int newScale){
    scaleFactor = defaultScale * float(newScale) / float(100);
    setImage();
}

void OverlapViewer::setBias(int newBias){
    biasFactor = float(newBias) / float(100);
    setImage();
}

void OverlapViewer::paintA(bool on){
    drawA = on;
    setImage();
}

void OverlapViewer::paintB(bool on){
    drawB = on;
    setImage();
}

void OverlapViewer::setImage(){
    if(!mergedImage){
	return;
    }
    // Put imageA in the middle of the merged image. Then offset imageB by dx and dy..
    
    // 1. work out the starting point for imageA
    
    int ox = (mergedWidth - imageWidth) / 2;
    int oy = (mergedHeight - imageHeight) / 2;

    // this may not be the fastest way of doing things, but maybe the simplest..
    // blank the background
    memset((void*)mergedImage, 0, mergedWidth * mergedHeight * 3 * sizeof(float));
    // and then add the area covered by two maps..
    for(uint y=0; y < imageHeight; y++){
	for(uint x=0; x < imageWidth; x++){
	    // imageA simply goes to the position ox + x, oy + y
	    // imageB maps to position ox + x + dx, oy + y + dy
	    // since B might map outside of the thingy we have to check the coordinates
	    // doing so here might be a bit slow, but ..
	    
	    // imageA maps to the red channel, and imageB maps to the green channel
	    if(drawA){
		mergedImage[3 * ((y + oy) * mergedWidth + x + ox)] = biasFactor + scaleFactor * imageA[y * imageWidth + x];
	    }
	    int by = y + oy + dy;
	    int bx = x + ox + dx;
//	    cout << "by, bx : " << by << ", " << bx << endl;
	    if(by > -1 && by < mergedHeight && bx > -1 && bx < mergedWidth && drawB){
		mergedImage[3 * (by * mergedWidth + bx) + 1] = biasFactor + scaleFactor * imageB[y * imageWidth + x];
	    }
	}
    }
    // then set this image in the middle of the first texture...
    //
//    int tx = (texSize - mergedWidth) / 2;
//    int ty = (texSize - mergedHeight) / 2;   // except the glImage doesn't yet take these coordinates.. hmm 
    glImage->setImage(mergedImage, mergedWidth, mergedHeight, 0, 0);
    glImage->updateGL();
}

float OverlapViewer::findMax(float* values, unsigned int length){
    if(!length){
	cerr << "OverlapViewer::findMax array has no length returning 1.0, and get bad results" << endl;
	return(1.0);
    }
    float max = values[0];
    for(uint i=1; i < length; i++){
	if(max < values[i]){ max = values[i]; }
    }
    return(max);
}



