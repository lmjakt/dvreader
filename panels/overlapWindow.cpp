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

#include "overlapWindow.h"
#include <iostream>
#include <qstring.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <string.h>
    

using namespace std;

OverlapWindow::OverlapWindow(vector<overlap_data*> olaps, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    data = olaps;
    int texSize = 1024;
    viewer = new OverlapViewer(texSize);
    viewer = new OverlapViewer(texSize, this, "viewer");
    viewer->resize(1024, 1024);
//     viewer->setCaption("Overlap Views");
//     viewer->show();
    setCaption("Overlap Window");

    QLabel* A_label = new QLabel("A", this, "A_label");
    QLabel* B_label = new QLabel("B", this, "B_label");

    panelLabel = new QLabel("0, 0", this, "panelLabel");
    neighborPanelLabel = new QLabel("0, 0", this, "neighbourPanelLabel");
    
    posLabel = new QLabel("0, 0", this, "posLabel");
    neighborPosLabel = new QLabel("0, 0", this, "neighbourPosLabel");

    corrLabel = new QLabel("-1.0", this, "corrLabel");
    normCorrLabel = new QLabel("-1.0", this, "corrLabel");
    offSetLabel = new QLabel("0, 0", this, "offsetLabel");

    QSpinBox* dataSpin = new QSpinBox(0, data.size()-1, 1, this);
    connect(dataSpin, SIGNAL(valueChanged(int)), this, SLOT(setOverlap(int)) );
    dataSpin->setValue(0);
    
    QHBoxLayout* box = new QHBoxLayout(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* grid = new QGridLayout(10, 2);
    box->addLayout(vbox);
    vbox->addLayout(grid);
    vbox->addStretch();
    grid->addWidget(A_label, 0, 0);
    grid->addWidget(posLabel, 1, 1);
    grid->addWidget(panelLabel, 2, 1);
    grid->addWidget(B_label, 3, 0);
    grid->addWidget(neighborPosLabel, 4, 1);
    grid->addWidget(neighborPanelLabel, 5, 1);
    grid->addWidget(dataSpin, 6, 1);
    grid->addMultiCellWidget(offSetLabel, 7, 7, 0, 1);
    grid->addMultiCellWidget(corrLabel, 8, 8, 0, 1);
    grid->addMultiCellWidget(normCorrLabel, 9, 9, 0, 1);

    box->addWidget(viewer);
    
}

void OverlapWindow::setOverlap(int n){
    if(n < 0 || n >= data.size()){
	cerr << "OverlapWindow::setOverlap inappropriate n given : " << n << "  data size is only " << data.size() << endl;
    }
    QString par1;
    QString par2;
    QString label;
    
    par1.setNum(data[n]->x);
    par2.setNum(data[n]->y);
    label = par1;
    label += ", ";
    label += par2;
    posLabel->setText(label);

    par1.setNum(data[n]->px);
    par2.setNum(data[n]->py);
    label = par1;
    label += ", ";
    label += par2;
    panelLabel->setText(label);

    par1.setNum(data[n]->nx);
    par2.setNum(data[n]->ny);
    label = par1;
    label += ", ";
    label += par2;
    neighborPosLabel->setText(label);

    par1.setNum(data[n]->pnx);
    par2.setNum(data[n]->pny);
    label = par1;
    label += ", ";
    label += par2;
    neighborPanelLabel->setText(label);

    par1.setNum(data[n]->offsts.dx);
    par2.setNum(data[n]->offsts.dy);
    label = par1;
    label += ", ";
    label += par2;
    offSetLabel->setText(label);

    par1.setNum(data[n]->offsts.corr);
    par2.setNum(data[n]->offsts.norm_corr);
    corrLabel->setText(par1);
    normCorrLabel->setText(par2);
    
    // and finally the important one..
    viewer->setImage(data[n]->a, data[n]->b, data[n]->width, data[n]->height, data[n]->dx, data[n]->dy);

}
