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

#include "setWidget.h"
#include "qlabel.h"
#include <qvalidator.h>
#include <iostream>

using namespace std;

SetWidget::SetWidget(QString buttonLabel, QWidget* parent, const char* name)
    : NucleusWidget(buttonLabel, parent, name)
{
    // Make a couple of new line Edits..
    minSizeLine = new QLineEdit(this);
    QIntValidator* minValidator = new QIntValidator(minSizeLine);
    minSizeLine->setValidator(minValidator);

    maxSizeLine = new QLineEdit(this);
    QIntValidator* maxValidator = new QIntValidator(maxSizeLine);
    maxSizeLine->setValidator(maxValidator);
    
    QLabel* minLabel = new QLabel("Min Size", this);
    QLabel* maxLabel = new QLabel("Max Size", this);
    
    hbox->addWidget(minLabel);
    hbox->addWidget(minSizeLine);
    hbox->addWidget(maxLabel);
    hbox->addWidget(maxSizeLine);
    
    // and the redo findNuclei.. 
}

void SetWidget::findNuclei(){
    cout << "Overriden function : findNuclei from SetWidget" << endl;
    bool ok;
    float minValue = minLine->text().toFloat(&ok);
    if(!ok){
	cerr << "SetWidget unable to parse float from minLine" << endl;
	return;
    }
    int minSize = minSizeLine->text().toInt(&ok);
    if(!ok){
	cerr << "SetWidget unable to parse int from minSizeLine" << endl;
	return;
    }
    int maxSize = maxSizeLine->text().toInt(&ok);
    if(!ok){
	cerr << "SetWidget unable to parese int from maxSizeLine" << endl;
	return;
    }
    if(minSize >= maxSize){
	cerr << "SetWidget minSize is >= to MaxSize makes no sense : " << minSize << " >= " << maxSize << endl;
	return;
    }
    int selectedId = channels->selectedId();
    if(selectedId == -1){
	cerr << "NucleusWidget : no wavelengt selected" << endl;
	return;
    }
    emit findSets(selectedId, minSize, maxSize, minValue);
}
