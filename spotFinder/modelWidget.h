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

#ifndef MODELWIDGET_H
#define MODELWIDGET_H

#include <vector>
#include <set>
#include <qwidget.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qstring.h>
//Added by qt3to4:
#include <QHBoxLayout>

using namespace std;       // which I know is bad, but signals and slots seems to have trouble without it 

class ModelWidget : public QWidget
{
    Q_OBJECT

	public :
	ModelWidget(int Width, int Height, int Depth, QWidget* parent=0, const char* name=0);   // give maximum depths and so on.. 
    void setChannels(vector<QString> Channels);
    
    signals :
	void makeModel(int, int, int, int, int, int, set<int>);  // x, width, y, height, z, depth, channels (full thingy..)
    void recalculateSpotVolumes();
    
    private :
	QHBoxLayout* channelBox;  // somewhere to keep the channels..
    vector<QCheckBox*> channelBoxes;
    int w, h, d;
    QSpinBox* xBeginBox;
    QSpinBox* widthBox;
    QSpinBox* yBeginBox;
    QSpinBox* heightBox;
    QSpinBox* zBeginBox;
    QSpinBox* depthBox;
    
    private slots :
	void requestModel(); 
};

    
    
    

#endif
