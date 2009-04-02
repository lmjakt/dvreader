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

#include "spotDensityWidget.h"
#include <QLabel>
#include <QPushButton>
#include <QLayout>

SpotDensityWidget::SpotDensityWidget(QWidget* parent)
    : QWidget(parent)
{
    radiusBox = new QSpinBox(this);
    sigmaBox = new QDoubleSpinBox(this);
    orderBox = new QDoubleSpinBox(this);
    
    radiusBox->setRange(1, 100);
    sigmaBox->setRange(1.0, 100);
    orderBox->setRange(1.1, 10);

    sigmaBox->setSingleStep(0.1);
    orderBox->setSingleStep(0.1);

    radiusBox->setValue(10);
    sigmaBox->setValue(20.0);
    orderBox->setValue(2.0);

    QLabel* r_label = new QLabel("radius", this);
    QLabel* s_label = new QLabel("sigma", this);
    QLabel* o_label = new QLabel("order", this);

    // and a QPushButton..
    QPushButton* d_button = new QPushButton("Calculate Density", this);
    connect(d_button, SIGNAL(clicked()), this, SLOT(findDensity()) );
    

    // and a layout..
    QHBoxLayout* h_box = new QHBoxLayout(this);
    h_box->setSpacing(1);
    h_box->addStretch();
    h_box->addWidget(r_label);
    h_box->addWidget(radiusBox);
    h_box->addWidget(s_label);
    h_box->addWidget(sigmaBox);
    h_box->addWidget(o_label);
    h_box->addWidget(orderBox);
    h_box->addWidget(d_button);
}

void SpotDensityWidget::findDensity(){
    emit findSpotDensity(radiusBox->value(), sigmaBox->value(), orderBox->value());
}
