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

#ifndef OVERLAPVIEWER_H
#define OVERLAPVIEWER_H

#include "../opengl/glImage.h"
#include <qwidget.h>
#include <qspinbox.h>
#include <qlabel.h>

class OverlapViewer : public QWidget
{
    Q_OBJECT

	public :
	OverlapViewer(unsigned int textureSize, QWidget* parent=0, const char* name=0);
    ~OverlapViewer();
    
    void setImage(float* a, float* b, unsigned int Width, unsigned int Height, int delta_x=0, int delta_y=0);
    
    private slots :
	void set_dx(int delta_x);
    void set_dy(int delta_y);
    void setScale(int newScale);
    void setBias(int newBias);    // these are read directly from the spin box, -but they get interpreted to float values.. 

    void paintA(bool on);
    void paintB(bool on);   // do we paint both or not.. 

    private :
	GLImage* glImage;
    QSpinBox* dxBox;
    QSpinBox* dyBox;
    QSpinBox* scaleBox;
    QSpinBox* biasBox;
    
    QLabel* scaleValueLabel;
    QLabel* biasValueLabel;

    float biasFactor;
    float scaleFactor;

    float defaultScale;   // set such that maxValue is 
    
    float* imageA;
    float* imageB;
    float* mergedImage;    // the merged image.. 

    int dx, dy;  // delta x and delta y.
    unsigned int imageWidth;
    unsigned int imageHeight;
    unsigned int mergedWidth;
    unsigned int mergedHeight;  
    unsigned int texSize;

    bool drawA, drawB;

    void setImage();    // makes the appropriate float and sets the image in the glImage..
    float findMax(float* values, unsigned int length);  // return the maxium value .. 

};

#endif
