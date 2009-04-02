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

#ifndef OVERLAPWINDOW_H
#define OVERLAPWINDOW_H


#include "overlapViewer.h"
#include "../dataStructs.h"
#include <qwidget.h>
//#include <qspinbox.h>
#include <qlabel.h>
#include <vector>

class OverlapWindow : public QWidget
{
    Q_OBJECT
	public :
	OverlapWindow(std::vector<overlap_data*> olaps, QWidget* parent=0, const char* name=0);

    private slots :
	void setOverlap(int n);

    private :
	OverlapViewer* viewer;
    QLabel* panelLabel;
    QLabel* neighborPanelLabel;
    QLabel* posLabel;
    QLabel* neighborPosLabel;

    QLabel* corrLabel;
    QLabel* normCorrLabel;
    QLabel* offSetLabel;
    
    std::vector<overlap_data*> data;

};


#endif
