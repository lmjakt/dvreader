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

#ifndef SPOTSWIDGET_H
#define SPOTSWIDGET_H

#include <map>
#include <qwidget.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <QPalette>
#include "../button/dropButton.h"
#include "../dataStructs.h"


class SpotsWidget : public QWidget
{
    Q_OBJECT
	
	public :
	SpotsWidget(QString waveLabel, int WaveLength, int WaveIndex, int r, float mpv, float mev,  threeDPeaks* Peaks, QWidget* parent=0, const char* name=0);
    ~SpotsWidget();
    
    void color(float& r, float& g, float& b);
    int wlength(){
	return(waveLength);
    }
    DropRepresentation dropType(){  // what kind of drop representation is selected.. 
	return(currentType);
    }
    threeDPeaks* peaks(){
	return(drops);
    }
    int w_index(){
	return(waveIndex);
    }
    void peakProperties(int& wl, int& r, float& mpv, float& mev){
	wl = waveLength; r = radius; mpv = minPeakValue; mev = maxEdgeValue;
    }

    private slots :
	void setColor();
    void repButtonToggled(bool on);

    private :
	std::map<DropRepresentation, DropButton*> dropButtons;  // these are thingy.. 
    threeDPeaks* drops;
    DropRepresentation currentType;
    QColor currentColor;
    QPushButton* colorButton;
    QString labelString;
    int waveLength;
    int waveIndex;
    int radius;
    float minPeakValue, maxEdgeValue;
    QPalette palette;

    signals :
	void colorChanged(int, float, float, float);
    void colorChanged();
    void repTypeChanged(DropRepresentation);   // when we toggle the buttons.. 
    void repTypeChanged();
};

#endif
