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

#ifndef SPOTSECION_H
#define SPOTSECTION_H

#include "spotSectionView.h"
#include "../dataStructs.h"
#include <qspinbox.h>

class SpotSection : public QWidget
{
    Q_OBJECT
	public :
	SpotSection(QWidget* parent=0, const char* name=0);

    void setDrop(simple_drop& drop);
    
    private slots :
	void setXYsection(int sec);
    void setXZsection(int sec);

    private :
	SpotSectionView* x_y_view;
    SpotSectionView* x_z_view;
    QSpinBox* XY_section;
    QSpinBox* XZ_section;

    simple_drop currentDrop;
};

#endif
