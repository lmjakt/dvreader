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

#ifndef NUCLEUSWIDGET_H
#define NUCLEUSWIDGET_H

#include <vector>
#include <qwidget.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>

class NucleusWidget : public QWidget
{
    Q_OBJECT
	
	public :
	NucleusWidget(QString buttonLabel, QWidget* parent=0, const char* name=0);
    void setChannels(std::vector<QString> Channels);

    signals :
	void findNuclearPerimeters(int, float);          // float -> minimum nuclear signal.. 

    protected :
	QVBoxLayout* vbox;
    QHBoxLayout* hbox;    // so I can do something reasonable .. 
    QLineEdit* minLine;       // input the minium line ..
    Q3ButtonGroup* channels;   // we use this to handle stuff.. but ..

    private :
	std::vector<QRadioButton*> channelButtons;

    protected slots :
	virtual void findNuclei();
};

#endif
