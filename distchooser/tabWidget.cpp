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

#include "tabWidget.h"
#include <qlayout.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QVBoxLayout>

TabWidget::TabWidget(QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    tabs = new QTabWidget(this, "tabs");
    
    //QPushButton* copy_button = new QPushButton("Copy", this, "copy_button");
    //QPushButton* paste_button = new QPushButton("Paste", this, "paste_button");
    //connect(copy_button, SIGNAL(clicked()), this, SIGNAL(copyRanges()) );
    //connect(paste_button, SIGNAL(clicked()), this, SIGNAL(pasteRanges()) );
    
    QVBoxLayout* vbox = new QVBoxLayout(this, 0, 0);
    vbox->addWidget(tabs);
    //QHBoxLayout* buttonBox = new QHBoxLayout(vbox, 0);
    //buttonBox->addStretch();
    //buttonBox->addWidget(copy_button);
    //buttonBox->addWidget(paste_button);
}

void TabWidget::addTab(QWidget* widget, QString label){
    tabs->addTab(widget, label);
    connect(widget, SIGNAL(copyRanges()), this, SIGNAL(copyRanges()) );
    connect(widget, SIGNAL(pasteRanges()), this, SIGNAL(pasteRanges()) );
    connect(widget, SIGNAL(saveRanges()), this, SIGNAL(saveRanges()) );
    connect(widget, SIGNAL(readRangesFromFile()), this, SIGNAL(readRangesFromFile()) );
}
