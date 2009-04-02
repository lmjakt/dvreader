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

#ifndef PARAMETERCHOOSER_H
#define PARAMETERCHOOSER_H

#include "colorChooser.h"
#include "dataStructs.h"
#include "distchooser/distChooser.h"


class ParameterChooser : public ColorChooser
{
    Q_OBJECT
	
	public :
	ParameterChooser(parameterData parData, QString label, int windex, int wlength, QColor c, QWidget* parent=0, const char* name=0);
    ~ParameterChooser();
    
    float biasValue(){
	return(bias);
    }
    float scaleValue(){
	return(scale);
    }

    private :
	parameterData data;
    DistChooser* chooser;
    float bias;
    float scale;
    
    private slots :
	void displayDistChooser();  // display and raise..
    void setBiasAndScale(float lowT, float highT);  // calculates the bias and the scale from the two thresholds.. 
    signals :
	void newRanges(float, float);
};

#endif
