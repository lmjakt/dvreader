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

#include "clusterWidget.h"
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QHBoxLayout>

ClusterWidget::ClusterWidget(int id, Cluster& clust, QWidget* parent, const char* name)
    : QWidget(parent, name)
{
    cluster = clust;
    identifier = id;
    QString id_string;
    id_string.setNum(identifier);
    QString size_string;
    size_string.setNum(cluster.members.size());
    QString md_string;
    md_string.setNum(cluster.maxDistance);
    
    // and a few labels to contain the things..
    int indent = 5;
    QLabel* idLabel = new QLabel(id_string, this, "idLabel");
    idLabel->setFixedWidth(40);
    QLabel* sizeLabel = new QLabel(size_string, this, "sizelabel");
    sizeLabel->setFixedWidth(50);
    QLabel* mdLabel = new QLabel(md_string, this, "mdLabel");
    mdLabel->setFixedWidth(75);

    idLabel->setIndent(indent);
    sizeLabel->setIndent(indent);
    mdLabel->setIndent(indent);

    // we also need two checkboxes that are connected to plot_members, and plot_centers..
    QCheckBox* memberBox = new QCheckBox("M", this, "memberBox");
    QCheckBox* centerBox = new QCheckBox("C", this, "centerBox");
    connect(memberBox, SIGNAL(toggled(bool)), this, SLOT(plot_members(bool)) );
    connect(centerBox, SIGNAL(toggled(bool)), this, SLOT(plot_center(bool)) );

    // we also need something to put these in.. for now just use a simple qhboxlayout
    QHBoxLayout* hbox = new QHBoxLayout(this, 3);    // let's have some distance between things..
    hbox->addStretch();
    hbox->addWidget(idLabel);
    hbox->addWidget(sizeLabel);
    hbox->addWidget(mdLabel);
    hbox->addWidget(centerBox);
    hbox->addWidget(memberBox);
    // and leave it at that. It will look a bit ugly, but,, 
}

void ClusterWidget::plot_members(bool on){
    if(!on){
	emit unPlotMembers(identifier);
	return;
    }
//     // ok, need to get the stuff..
//     vector<vector<float> > lines(cluster.members.size());
//     for(uint i=0; i < lines.size(); i++){
// 	lines[i] = cluster.members[i].values;
//     }
    emit plotMembers(identifier, cluster.members);
}

void ClusterWidget::plot_center(bool on){
    if(!on){
	emit unPlotCenter(identifier);
	return;
    }
    emit plotCenter(identifier, cluster.center);
}

    
